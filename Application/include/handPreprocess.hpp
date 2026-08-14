#pragma once

#include <vector>
#include <SDL3/SDL.h>

class HandPreprocess {
    private:
        static constexpr int inputSize = 224;

        SDL_Surface* cropSurface(SDL_Surface* frame, SDL_Rect crop);
        SDL_Surface* resizeSurface(SDL_Surface* surface);
        std::vector<float> normalize(SDL_Surface* surface);

    public:
        SDL_Rect getCenterCrop(SDL_Surface* frame) const;

        std::vector<float> process(SDL_Surface* frame, SDL_Rect crop);
        SDL_Rect createSquareCrop(float centerX, float centerY, float size, int frameWidth, int frameHeight);
};