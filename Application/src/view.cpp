#include "model.hpp"
#include "view.hpp"

#include "renderImGUI.hpp"
#include "renderSDL.hpp"


View::View() {
    sdlRenderer = new RenderSDL();
    imguiRenderer = new RenderImGUI();
}

View::~View() {
    delete sdlRenderer;
    delete imguiRenderer;
}

void View::setHoverIndex(int index) {
    hoverIndex = index;
}

void View::init(SDL_Renderer* renderer) {
    sdlRenderer->init(renderer);
    imguiRenderer->setRenderer(sdlRenderer);
}

void View::render(SDL_Window* window, SDL_Renderer* renderer, Model& model) {
    // Render both scene and UI
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    sdlRenderer->render(model, hoverIndex, windowWidth, windowHeight);
    imguiRenderer->render(model);
    SDL_RenderPresent(renderer);
}