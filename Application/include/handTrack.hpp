#pragma once

#include <array>
#include <mutex>
#include <thread>
#include <atomic>
#include <SDL3/SDL.h>

#include "onnxModel.hpp"
#include "oneEuroFilter.hpp"
#include "handPreprocess.hpp"

struct Joint {
    float x;
    float y;
};

struct BBox {
    float x;
    float y;
    float w;
    float h;
};

class HandTracker {
    private:
        ONNXModel detector;
        ONNXModel landmark;

        HandPreprocess processor;

        std::array<Joint, 21> joints{};
        BBox bbox{};

        mutable std::mutex resultMutex;
        std::atomic<bool> handDetected{false};

        OneEuroFilter filterBx{2.0f, 0.1f, 1.0f};
        OneEuroFilter filterBy{2.0f, 0.1f, 1.0f};
        OneEuroFilter filterBw{2.0f, 0.1f, 1.0f};
        OneEuroFilter filterBh{2.0f, 0.1f, 1.0f};

        std::array<OneEuroFilter, 21> filterX;
        std::array<OneEuroFilter, 21> filterY;

        std::thread thread;
        std::mutex frameMutex;

        SDL_Surface* pendingFrame = nullptr;

        std::atomic<bool> running{false};
        std::atomic<bool> newFrame{false};

        static constexpr float filterMinCutoff = 1.0f;
        static constexpr float filterBeta = 0.4f;
        static constexpr float filterDCutoff = 1.0f;
        static constexpr float bboxScale = 1.75f;

        void loop();
        void update(const std::array<Joint, 21>& joints, BBox bbox);
        BBox detectHand(SDL_Surface* frame);
        std::array<Joint, 21> detectLandmarks(SDL_Surface* frame, BBox bbox);
        BBox expandBBox(BBox box, float scale);

    public:
        HandTracker(const char* detectorPath, const char* landmarkPath);
        ~HandTracker();

        std::array<Joint, 21> getJoints() const;
        SDL_Surface* getFrame();
        BBox getBBox() const;

        void start();
        void stop();
        void submitFrame(SDL_Surface* frame);
        bool isHandDetected() const;
};