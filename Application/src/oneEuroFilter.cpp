#include <cmath>

#include "oneEuroFilter.hpp"


OneEuroFilter::OneEuroFilter(float minCutoff, float beta, float dCutoff)
    : minCutoff(minCutoff), beta(beta), dCutoff(dCutoff), xPrev(0.0f), dxPrev(0.0f), initialized(false) {
}

float OneEuroFilter::filter(float x, float dt) {
    // Initialize filter state
    if (dt <= 0.0f)
        dt = 1.0f / 60.0f;

    if (!initialized) {
        xPrev = x;
        dxPrev = 0.0f;
        initialized = true;
        return x;
    }

    // Filter input value
    float dx = (x - xPrev) / dt;
    float dxEstimate = lowPass(dx, dxPrev, alpha(dt, dCutoff));

    float cutoff = minCutoff + beta * std::fabs(dxEstimate);
    float xFiltered = lowPass(x, xPrev, alpha(dt, cutoff));

    xPrev = xFiltered;
    dxPrev = dxEstimate;

    return xFiltered;
}

void OneEuroFilter::reset() {
    initialized = false;
    xPrev = 0.0f;
    dxPrev = 0.0f;
}


// Private Helper Functions

float OneEuroFilter::alpha(float dt, float cutoff) const {
    // Compute the smoothing factor
    float tau = 1.0f / (2.0f * M_PI * cutoff);
    return 1.0f / (1.0f + tau / dt);
}

float OneEuroFilter::lowPass(float x, float previous, float alpha) const {
    // Apply low pass filter
    return alpha * x + (1.0f - alpha) * previous;
}