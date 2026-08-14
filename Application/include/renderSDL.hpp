#pragma once

#include <array>
#include <SDL3/SDL.h>
#include <SDL3/SDL_camera.h>

class Model;
struct Joint;

class RenderSDL {
    private:
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* circleTexture = nullptr;
        SDL_Camera* camera = nullptr;
        SDL_Texture* cameraTexture = nullptr;

        bool cameraEnabled = false;
        bool handTrackingEnabled = false;
        bool displayJoints = false;
        bool displayBBox = false;

        static constexpr float cameraX = 0.0f;
        static constexpr float cameraY = 0.0f;
        static constexpr float cameraWidth = 224.0f;
        static constexpr float cameraHeight = 224.0f;

        static constexpr float size = 7.5f;
        static constexpr float jointSize = 3.0f;
        static constexpr int circleTextureSize = 512;

        SDL_Texture* createCircleTexture(int size);

        void drawCircles(Model& model, int hoverIndex);
        void drawCamera(Model& model, float x, float y, float width, float height);
        void drawJoints(Model& model, float x, float y, float width, float height);
        void drawBBox(Model& model, float x, float y, float width, float height);
        void drawCursor(Model& model);

    public:
        ~RenderSDL();

        SDL_Renderer* getRenderer() const;

        void init(SDL_Renderer* renderer);
        void render(Model& model, int hoverIndex, int windowWidth, int windowHeight);

        void toggleCamera();
        void toggleHandTracking();
        void toggleDisplayJoints();
        void toggleDisplayBBox();

        bool isCameraEnabled() const;
        bool isHandTrackingEnabled() const;
        bool isDisplayJointsEnabled() const;
        bool isDisplayBBoxEnabled() const;
};