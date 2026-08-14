#pragma once

#include <vector>
#include <array>
#include <chrono>

#include "handTrack.hpp"

struct Circle {
    float x, y;
    float radius;
    float r, g, b;
    float mass;
    float vx, vy;
};

enum class Gesture {
    None,
    TwoFingerPoint,
    Fist
};

class Model {
    private:
        std::vector<Circle> circles;
        HandTracker handTracker;

        Gesture gesture = Gesture::None;
        Gesture previousGesture = Gesture::None;

        int handSelected = -1;
        int handGrabbed = -1;

        int lastHandSelected = -1;
        int handGraceFrames = 0;

        float handX = 0.0f;
        float handY = 0.0f;

        float prevHandX = 0.0f;
        float prevHandY = 0.0f;

        float handVx = 0.0f;
        float handVy = 0.0f;

        OneEuroFilter handFilterX{0.8f, 0.8f, 1.0f};
        OneEuroFilter handFilterY{0.8f, 0.8f, 1.0f};

        std::chrono::steady_clock::time_point lastFrameTime;
        bool firstFrame = true;

        static constexpr int graceLimit = 8;
        static constexpr float throwScale = 2.0f;
        static constexpr float selectRadius = 1.3f;
        static constexpr float fingerThreshold = 0.055f;
        
        static constexpr float cursorSensitivity = 0.9f;
        static constexpr float cursorVelocitySmoothing = 0.4f;
        static constexpr float cursorVelocityDecay = 0.6f;
        
        static constexpr float friction = 0.99f;
        static constexpr float bounce = 0.8f;
        static constexpr float restitution = 0.7f;
        static constexpr float minimumDistance = 0.01f;
        static constexpr int collisionIterations = 4;

        bool isFingerExtended(const std::array<Joint, 21>& joints, int tip, int base) const;
        Gesture detectGesture(const std::array<Joint, 21>& joints) const;

        void updateHandCursor(const std::array<Joint, 21>& joints, int width, int height);
        void updateHandVelocity();

        void updateSelection();
        void grabCircle();
        void moveGrabbedCircle();
        void releaseCircle();

        void keepCircleInside(Circle& circle, int width, int height, float arenaWidth);
        bool checkCollision(Circle& a, Circle& b, float& nx, float& ny, float& overlap);
        void resolvePosition(Circle& a, Circle& b, float nx, float ny, float overlap, int indexA, int indexB);
        void resolveVelocity(Circle& a, Circle& b, float nx, float ny, int indexA, int indexB);

    public:
        Model();
        ~Model();

        std::array<Joint, 21> getJoints() const;
        std::vector<Circle>& getCircles();
        BBox getBBox() const;
        Gesture getGesture() const;
        int getHandSelected() const;
        int getHandGrabbed() const;
        float getHandX() const;
        float getHandY() const;

        void setVelocity(int index, float vx, float vy);

        void addCircle(float x, float y, float radius, float r, float g, float b);
        void removeCircle();
        void moveCircle(int index, float x, float y, int width, int height, float arenaWidth);

        void updatePhysics(float dt, int width, int height, float arenaWidth);
        void collisions(int width, int height, float arenaWidth);

        void updateHandTracking(SDL_Surface* frame);
        void updateGesture(int width, int height);
        bool isHandDetected() const;
};