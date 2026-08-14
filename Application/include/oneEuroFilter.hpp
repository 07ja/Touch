#pragma once

class OneEuroFilter {
    private:
        float minCutoff;
        float beta;
        float dCutoff;

        float xPrev;
        float dxPrev;
        bool initialized;

        float alpha(float dt, float cutoff) const;
        float lowPass(float x, float previous, float alpha) const;

    public:
        OneEuroFilter(float minCutoff = 1.0f, float beta = 0.0f, float dCutoff = 1.0f);
        
        float filter(float x, float dt);
        void reset();
};