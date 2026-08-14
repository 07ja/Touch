#include <iostream>

#include "controller.hpp"
#include "model.hpp"
#include "view.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"


Controller::Controller() {
}

Controller::~Controller() { 
    destroy();
}

SDL_Renderer* Controller::getRenderer() const { 
    return renderer;
}

SDL_Window* Controller::getWindow() const { 
    return window;
}

bool Controller::getQuit() const { 
    return quit;
}

void Controller::setWindowSize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

void Controller::init(const char* title, int width, int height) {
    // Initialize SDL window and renderer
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA))
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;

    window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    if (!window)
        std::cerr << "Window Creation Failed: " << SDL_GetError() << std::endl;

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
        std::cerr << "Renderer Creation Failed: " << SDL_GetError() << std::endl;
    
    // Configure renderer and window
    SDL_SetRenderVSync(renderer, 1);
    setWindowSize(width, height);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

void Controller::input(View& view, Model& model) {
    SDL_Event event;

    // Process input events
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        handleWindowEvent(event);
        handleMouseDown(event, model);
        handleMouseMotion(event, view, model);
        handleMouseUp(event, model);
    }

    model.updatePhysics(1.0f, windowWidth, windowHeight, arenaWidth);
    model.collisions(windowWidth, windowHeight, arenaWidth);

    if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_ESCAPE])
        quit = true;
}

void Controller::destroy() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


// Private Helper Functions

void Controller::handleWindowEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_QUIT)
        quit = true;

    if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        SDL_Window* resizedWindow = SDL_GetWindowFromID(event.window.windowID);
        SDL_GetWindowSize(resizedWindow, &windowWidth, &windowHeight);
    }
}

void Controller::handleMouseDown(const SDL_Event& event, Model& model) {
    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN || event.button.button != SDL_BUTTON_LEFT || ImGui::GetIO().WantCaptureMouse)
        return;

    std::vector<Circle>& circles = model.getCircles();

    // Check for selected circle
    for (int i = 0; i < (int)(circles.size()); i++) {
        float dx = event.button.x - circles[i].x;
        float dy = event.button.y - circles[i].y;

        if (dx * dx + dy * dy < circles[i].radius * circles[i].radius) {
            mouseGrabbed = i;
            mouseOffsetX = dx;
            mouseOffsetY = dy;
            break;
        }
    }
}

void Controller::handleMouseMotion(const SDL_Event& event, View& view, Model& model) {
    if (event.type != SDL_EVENT_MOUSE_MOTION || ImGui::GetIO().WantCaptureMouse)
        return;

    std::vector<Circle>& circles = model.getCircles();

    // Update hovered circle
    int hover = -1;
    for (int i = 0; i < (int)(circles.size()); i++) {
        float dx = event.motion.x - circles[i].x;
        float dy = event.motion.y - circles[i].y;

        if (dx * dx + dy * dy < circles[i].radius * circles[i].radius) {
            hover = i;
            break;
        }
    }

    view.setHoverIndex(hover);

    // Drag selected circle
    if (mouseGrabbed != -1) {
        float newX = event.motion.x - mouseOffsetX;
        float newY = event.motion.y - mouseOffsetY;

        mouseVx = event.motion.x - prevMouseX;
        mouseVy = event.motion.y - prevMouseY;

        model.moveCircle(mouseGrabbed, newX, newY, windowWidth, windowHeight, arenaWidth);
        model.setVelocity(mouseGrabbed, 0.0f, 0.0f);
    }

    prevMouseX = event.motion.x;
    prevMouseY = event.motion.y;
}

void Controller::handleMouseUp(const SDL_Event& event, Model& model) {
    if (event.type != SDL_EVENT_MOUSE_BUTTON_UP || event.button.button != SDL_BUTTON_LEFT)
        return;

    // Apply release velocity
    if (mouseGrabbed != -1)
        model.setVelocity(mouseGrabbed, mouseVx * 0.5f, mouseVy * 0.5f);

    mouseGrabbed = -1;
}