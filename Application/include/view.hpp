#pragma once

#include <SDL3/SDL.h>

class Model;
class RenderSDL;
class RenderImGUI;

class View {
    private:
        RenderSDL* sdlRenderer = nullptr;
        RenderImGUI* imguiRenderer = nullptr;

        int hoverIndex = -1;

    public:
        View();
        ~View();

        void setHoverIndex(int index);

        void init(SDL_Renderer* renderer);
        void render(SDL_Window* window, SDL_Renderer* renderer, Model& model);
};