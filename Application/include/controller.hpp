#pragma once

#include <SDL3/SDL.h>

class View;
class Model;

class Controller {
    private:
        SDL_Renderer* renderer = nullptr;
        SDL_Window* window = nullptr;

        bool quit = false;

        int windowWidth = 960;
        int windowHeight = 540;

        float mouseOffsetX = 0.0f;
        float mouseOffsetY = 0.0f;

        float prevMouseX = 0.0f;
        float prevMouseY = 0.0f;

        float mouseVx = 0.0f;
        float mouseVy = 0.0f;

        float arenaWidth = 224.0f;

        int mouseGrabbed = -1;

        void handleWindowEvent(const SDL_Event& event);
        void handleMouseDown(const SDL_Event& event, Model& model);
        void handleMouseMotion(const SDL_Event& event, View& view, Model& model);
        void handleMouseUp(const SDL_Event& event, Model& model);

    public:
        Controller();
        ~Controller();

        SDL_Renderer* getRenderer() const;
        SDL_Window* getWindow() const;
        bool getQuit() const;

        void setWindowSize(int width, int height);
        
        void init(const char* title, int width, int height);
        void input(View& view, Model& model);
        void destroy();
};