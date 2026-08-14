#pragma once

class Model;
class RenderSDL;

class RenderImGUI {
    private:
        RenderSDL* sdlRenderer = nullptr;

        static constexpr float buttonWidth = 100.0f;
        static constexpr float panelWidth = 224.0f;

        static constexpr float cameraBottom = 224.0f;
        static constexpr float arenaWidth = 224.0f;
        static constexpr float gap = 8.0f;

        static constexpr float spawnPadding = 50.0f;
        static constexpr float minRadius = 20.0f;
        static constexpr float maxRadius = 37.0f;

    public:
        void setRenderer(RenderSDL* renderer);
        
        void render(Model& model);
};