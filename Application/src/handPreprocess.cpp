#include <algorithm>

#include "handPreprocess.hpp"


SDL_Rect HandPreprocess::getCenterCrop(SDL_Surface* frame) const {
    SDL_Rect crop{};

    if (!frame)
        return crop;

    int size = std::min(frame->w, frame->h);

    // Calculate centered square crop
    crop.x = (frame->w - size) / 2;
    crop.y = (frame->h - size) / 2;
    crop.w = size;
    crop.h = size;

    return crop;
}

std::vector<float> HandPreprocess::process(SDL_Surface* frame, SDL_Rect crop) {
    // Input buffer
    std::vector<float> input(3 * inputSize * inputSize, 0.0f);

    if (!frame)
        return input;

    SDL_Surface* cropped = cropSurface(frame, crop);

    if (!cropped)
        return input;

    // Resize cropped image and normalize
    SDL_Surface* resized = resizeSurface(cropped);

    if (!resized) {
        SDL_DestroySurface(cropped);
        return input;
    }

    input = normalize(resized);

    SDL_DestroySurface(cropped);
    SDL_DestroySurface(resized);

    return input;
}

SDL_Rect HandPreprocess::createSquareCrop(float centerX, float centerY, float size, int frameWidth, int frameHeight) {
    // Create bounded square crop
    int cropSize = (int)(size);
    cropSize = std::min(cropSize, std::min(frameWidth, frameHeight));

    int x = (int)(centerX - cropSize / 2);
    int y = (int)(centerY - cropSize / 2);

    x = std::max(0, std::min(x, frameWidth - cropSize));
    y = std::max(0, std::min(y, frameHeight - cropSize));

    return {x, y, cropSize, cropSize};
}


// Private Helper Functions

SDL_Surface* HandPreprocess::cropSurface(SDL_Surface* frame, SDL_Rect crop) {
    if (!frame)
        return nullptr;

    SDL_Surface* converted = SDL_ConvertSurface(frame, SDL_PIXELFORMAT_RGBA32);

    if (!converted)
        return nullptr;

    // Clamp crop boundaries and validate size
    crop.x = std::max(0, crop.x);
    crop.y = std::max(0, crop.y);

    crop.w = std::min(crop.w, converted->w - crop.x);
    crop.h = std::min(crop.h, converted->h - crop.y);

    if (crop.w <= 0 || crop.h <= 0) {
        SDL_DestroySurface(converted);
        return nullptr;
    }

    // Create the cropped surface
    SDL_Surface* result = SDL_CreateSurface(crop.w, crop.h, SDL_PIXELFORMAT_RGBA32);

    if (!result) {
        SDL_DestroySurface(converted);
        return nullptr;
    }

    SDL_BlitSurface(converted, &crop, result, nullptr);
    SDL_DestroySurface(converted);

    return result;
}

SDL_Surface* HandPreprocess::resizeSurface(SDL_Surface* surface) {
    // Resize the image to the model input dimensions
    if (!surface)
        return nullptr;

    SDL_Surface* resized = SDL_CreateSurface(inputSize, inputSize, SDL_PIXELFORMAT_RGBA32);

    if (!resized)
        return nullptr;

    SDL_BlitSurfaceScaled(surface, nullptr, resized, nullptr, SDL_SCALEMODE_LINEAR);

    return resized;
}

std::vector<float> HandPreprocess::normalize(SDL_Surface* surface) {
    // Create normalized input buffer
    std::vector<float> input(3 * inputSize * inputSize, 0.0f);

    if (!surface)
        return input;

    // Access image pixel data
    const SDL_PixelFormatDetails* format = SDL_GetPixelFormatDetails(surface->format);
    Uint32* pixels = (Uint32*)surface->pixels;
    int pitch = surface->pitch / 4;

    // Convert pixels to normalized RGB values
    for (int y = 0; y < inputSize; y++) {
        for (int x = 0; x < inputSize; x++) {

            Uint8 r, g, b, a;
            SDL_GetRGBA(pixels[y * pitch + x], format, nullptr, &r, &g, &b, &a);

            // Store normalized channel values
            int index = y * inputSize + x;
            input[index] = (r / 255.0f - 0.5f) / 0.5f;
            input[inputSize * inputSize + index] = (g / 255.0f - 0.5f) / 0.5f;
            input[2 * inputSize * inputSize + index] = (b / 255.0f - 0.5f) / 0.5f;
        }
    }

    return input;
}