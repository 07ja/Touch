#include <cmath>
#include <iostream>
#include <vector>

#include "renderSDL.hpp"
#include "model.hpp"


RenderSDL::~RenderSDL() {
    if (camera)
        SDL_CloseCamera(camera);

    if (cameraTexture)
        SDL_DestroyTexture(cameraTexture);

    if (circleTexture)
        SDL_DestroyTexture(circleTexture);
}

SDL_Renderer* RenderSDL::getRenderer() const {
    return renderer;
}

void RenderSDL::init(SDL_Renderer* r) {
    // Initialize renderer and textures
    renderer = r;
    circleTexture = createCircleTexture(circleTextureSize);
}

void RenderSDL::render(Model& model, int hoverIndex, int windowWidth, int windowHeight) {
    // Clear render target and draw background color
    SDL_SetRenderDrawColor(renderer, 28, 36, 27, 255);
    SDL_RenderClear(renderer);

    // Update hand gesture state
    if (handTrackingEnabled && model.isHandDetected())
        model.updateGesture(windowWidth, windowHeight);

    // Draw scene elements
    drawCircles(model, hoverIndex);
    drawCamera(model, cameraX, cameraY, cameraWidth, cameraHeight);

    if (handTrackingEnabled && model.isHandDetected())
        drawCursor(model);
}

void RenderSDL::toggleCamera() {
    cameraEnabled = !cameraEnabled;

    // Open camera device
    if (cameraEnabled) {
        int count = 0;

        SDL_CameraID* devices = SDL_GetCameras(&count);

        // Select first camera
        if (count > 0) {
            camera = SDL_OpenCamera(devices[0], nullptr);

            if (!camera) 
                cameraEnabled = false;
        }
        else
            cameraEnabled = false;
        if (devices)
            SDL_free(devices);
    }

    // Close camera device
    else {
        if (camera) {
            SDL_CloseCamera(camera);
            camera = nullptr;
        }

        if (cameraTexture) {
            SDL_DestroyTexture(cameraTexture);
            cameraTexture = nullptr;
        }
    }
}

void RenderSDL::toggleHandTracking() {
    handTrackingEnabled = !handTrackingEnabled;
}

void RenderSDL::toggleDisplayJoints() {
    displayJoints = !displayJoints;
}

void RenderSDL::toggleDisplayBBox() {
    displayBBox = !displayBBox;
}

bool RenderSDL::isCameraEnabled() const {
    return cameraEnabled;
}

bool RenderSDL::isHandTrackingEnabled() const {
    return handTrackingEnabled;
}

bool RenderSDL::isDisplayJointsEnabled() const {
    return displayJoints;
}

bool RenderSDL::isDisplayBBoxEnabled() const {
    return displayBBox;
}


// Private Helper Functions

SDL_Texture* RenderSDL::createCircleTexture(int size) {
    // Create circular texture
    SDL_Surface* surface = SDL_CreateSurface(size, size, SDL_PIXELFORMAT_RGBA32);

    if (!surface)
        return nullptr;

    float center = (size - 1) / 2.0f;
    float radius = size / 2.0f;
    float innerRadius = radius - 1.0f;

    Uint32* pixels = (Uint32*)surface->pixels;
    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surface->format);

    // Generate circle pixels
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float dx = x - center;
            float dy = y - center;
            float distance = sqrtf(dx * dx + dy * dy);

            Uint8 alpha = 0;

            if (distance <= innerRadius)
                alpha = 255;
            else if (distance < radius)
                alpha = (Uint8)(((radius - distance) / (radius - innerRadius)) * 255.0f);

            pixels[y * size + x] = SDL_MapRGBA(details, nullptr, 255, 255, 255, alpha);
        }
    }

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (texture) {
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    return texture;
}

void RenderSDL::drawCircles(Model& model, int hoverIndex) {
    // Get circle objects
    std::vector<Circle>& circles = model.getCircles();

    int selected = model.getHandSelected();
    int grabbed = model.getHandGrabbed();

    // Render each circle
    for (int i = 0; i < (int)circles.size(); i++) {
        Circle& circle = circles[i];

        // Draw the circles highlight
        if (i == hoverIndex || i == selected || i == grabbed) {
            float size = (circle.radius + 1.0f) * 2.0f;

            SDL_FRect outline = {circle.x - circle.radius - 1.0f, circle.y - circle.radius - 1.0f, size, size};

            SDL_SetTextureColorMod(circleTexture, 255, 255, 255);
            SDL_RenderTexture(renderer, circleTexture, nullptr, &outline);
        }

        // Draw the circle texture
        float diameter = circle.radius * 2.0f;

        SDL_FRect rect = {circle.x - circle.radius, circle.y - circle.radius, diameter, diameter};

        SDL_SetTextureColorMod(circleTexture, (Uint8)(circle.r * 255.0f), (Uint8)(circle.g * 255.0f), (Uint8)(circle.b * 255.0f));
        SDL_RenderTexture(renderer, circleTexture, nullptr, &rect);
    }
}

void RenderSDL::drawCamera(Model& model, float x, float y, float width, float height) {
    if (!cameraEnabled || !camera)
        return;

    Uint64 timestamp = 0;
    SDL_Surface* frame = SDL_AcquireCameraFrame(camera, &timestamp);

    // Capture camera frame
    if (frame) {
        SDL_Surface* converted = SDL_ConvertSurface(frame, SDL_PIXELFORMAT_RGBA32);

        // Convert camera format
        if (converted) {
            if (!cameraTexture)
                cameraTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, converted->w, converted->h);

            SDL_UpdateTexture(cameraTexture, nullptr, converted->pixels, converted->pitch);

            if (handTrackingEnabled)
                model.updateHandTracking(converted);

            SDL_DestroySurface(converted);
        }
        SDL_ReleaseCameraFrame(camera, frame);
    }

    if (!cameraTexture)
        return;

    float texWidth;
    float texHeight;

    // Texture dimensions
    SDL_GetTextureSize(cameraTexture, &texWidth, &texHeight);

    float size = texWidth < texHeight ? texWidth : texHeight;
    SDL_FRect source = {(texWidth - size) / 2.0f, (texHeight - size) / 2.0f, size, size};
    SDL_FRect destination = {x, y, width, height};

    SDL_RenderTextureRotated(renderer, cameraTexture, &source, &destination, 0.0, nullptr, SDL_FLIP_HORIZONTAL);

    // Draw detection bounding boxes and joints
    if (displayBBox && model.isHandDetected())
        drawBBox(model, x, y, width, height);

    if (displayJoints && model.isHandDetected())
        drawJoints(model, x, y, width, height);
}

void RenderSDL::drawJoints(Model& model, float x, float y, float width, float height) {
    // Get the hand joints
    std::array<Joint, 21> joints = model.getJoints();
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

    // Draw joints
    for (int i = 0; i < 21; i++) {
        float px = x + (1.0f - joints[i].x) * width;
        float py = y + joints[i].y * height;

        SDL_FRect dot = {px - jointSize, py - jointSize, jointSize, jointSize};

        SDL_RenderFillRect(renderer, &dot);
    }
}

void RenderSDL::drawBBox(Model& model, float x, float y, float width, float height) {
    if (!model.isHandDetected())
        return;

    // Get the hand bounding box
    BBox bbox = model.getBBox();

    float x1 = x + (1.0f - bbox.x - bbox.w) * width;
    float y1 = y + bbox.y * height;
    float w = bbox.w * width;
    float h = bbox.h * height;

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

    SDL_FRect rect = {x1, y1, w, h};
    SDL_RenderRect(renderer, &rect);
}

void RenderSDL::drawCursor(Model& model) {
    // Get the current gesture
    Gesture gesture = model.getGesture();

    if (gesture != Gesture::TwoFingerPoint)
        return;

    float x = model.getHandX();
    float y = model.getHandY();

    // Draw hand tracking cursor crosshair
    SDL_SetRenderDrawColor(renderer, 255, 225, 255, 255);

    SDL_FRect horizontal = {x - size, y - 1.0f, size * 2.0f, 2.0f};
    SDL_FRect vertical = {x - 1.0f, y - size, 2.0f, size * 2.0f};

    SDL_RenderFillRect(renderer, &horizontal);
    SDL_RenderFillRect(renderer, &vertical);
}