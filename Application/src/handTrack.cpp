#include <algorithm>

#include "handTrack.hpp"


HandTracker::HandTracker(const char* detectorPath, const char* landmarkPath)
    :detector(detectorPath, "input", "bbox"), landmark(landmarkPath, "input","uv") {

    // Initialize joint filters for smoothing landmark predictions
    for(int i = 0; i < 21; i++) {
        filterX[i] = OneEuroFilter(filterMinCutoff, filterBeta, filterDCutoff);
        filterY[i] = OneEuroFilter(filterMinCutoff, filterBeta, filterDCutoff);
    }
}

HandTracker::~HandTracker() {
    stop();
}

std::array<Joint, 21> HandTracker::getJoints() const {
    // Return tracked joints safely
    std::lock_guard<std::mutex> lock(resultMutex);
    return joints;
}

SDL_Surface* HandTracker::getFrame() {
    // Retrieve pending frame
    std::lock_guard<std::mutex> lock(frameMutex);

    if(!newFrame)
        return nullptr;

    SDL_Surface* frame = pendingFrame;

    pendingFrame = nullptr;
    newFrame = false;

    return frame;
}

BBox HandTracker::getBBox() const {
    // Return hand bounding box safely
    std::lock_guard<std::mutex> lock(resultMutex);
    return bbox;
}

void HandTracker::start() {
    // Start tracking thread
    running = true;
    thread = std::thread(&HandTracker::loop,this);
}

void HandTracker::stop() {
    // Stop tracking thread
    running = false;

    if(thread.joinable())
        thread.join();

    if(pendingFrame)
        SDL_DestroySurface(pendingFrame);
}

void HandTracker::submitFrame(SDL_Surface* frame) {
    // Queue new camera frame
    if(!frame)
        return;

    SDL_Surface* copy = SDL_ConvertSurface(frame, SDL_PIXELFORMAT_RGBA32);

    std::lock_guard<std::mutex> lock(frameMutex);

    if(pendingFrame)
        SDL_DestroySurface(pendingFrame);

    pendingFrame = copy;
    newFrame = true;
}

bool HandTracker::isHandDetected() const {
    return handDetected;
}


// Private Helper Functions

void HandTracker::loop() {
    // Process tracking loop
    while(running) {
        SDL_Surface* frame = getFrame();

        if(!frame) {
            SDL_Delay(1);
            continue;
        }

        // Detect hand data
        BBox box = detectHand(frame);
        box = expandBBox(box, bboxScale);
        auto joints = detectLandmarks(frame, box);
        update(joints, box);

        SDL_DestroySurface(frame);
    }
}

BBox HandTracker::detectHand(SDL_Surface* frame) {
    // Detect hand bounding box
    SDL_Rect crop = processor.getCenterCrop(frame);

    auto input = processor.process(frame, crop);
    auto output = detector.run(input, {1, 3, 224, 224} );

    // Filter bounding box
    BBox result;
    result.x = filterBx.filter(output[0], 1.0f / 60.0f);
    result.y = filterBy.filter(output[1], 1.0f / 60.0f);
    result.w = filterBw.filter(output[2], 1.0f / 60.0f);
    result.h = filterBh.filter(output[3], 1.0f / 60.0f);

    return result;
}

std::array<Joint, 21> HandTracker::detectLandmarks(SDL_Surface* frame, BBox bbox) {
    // Detect hand landmarks
    std::array<Joint, 21> result{};

    int size = std::min(frame->w, frame->h);

    int offsetX = (frame->w - size) / 2;
    int offsetY = (frame->h - size) / 2;

    float bx = bbox.x * size + offsetX;
    float by = bbox.y * size + offsetY;
    float bw = bbox.w * size;
    float bh = bbox.h * size;

    float cx = bx + bw / 2.0f;
    float cy = by + bh / 2.0f;

    // Create landmark crop
    float cropSize = std::max(bw, bh) * 2.0f;
    SDL_Rect crop = processor.createSquareCrop(cx, cy, cropSize, frame->w, frame->h);

    auto input = processor.process(frame, crop);
    auto output = landmark.run(input, {1, 3, 224, 224});

    // Convert the joint coordinates
    for(int i = 0; i < 21; i++) {
        float rawX = std::clamp(output[i * 2], 0.0f, 1.0f);
        float rawY = std::clamp(output[i * 2 + 1], 0.0f, 1.0f);

        float pixelX = crop.x + rawX * crop.w;
        float pixelY = crop.y + rawY * crop.h;
        float normX = (pixelX - offsetX) / size;
        float normY = (pixelY - offsetY) / size;

        result[i].x = filterX[i].filter(normX, 1.0f / 60.0f);
        result[i].y = filterY[i].filter(normY, 1.0f / 60.0f);
    }

    return result;
}

void HandTracker::update(const std::array<Joint, 21>& newJoints, BBox newBox) {
    // Update tracking results
    std::lock_guard<std::mutex> lock(resultMutex);

    joints = newJoints;
    bbox = newBox;
    handDetected = true;
}

BBox HandTracker::expandBBox(BBox box, float scale) {
    // Expand the bounding box
    float centerX = box.x + box.w / 2.0f;
    float centerY = box.y + box.h / 2.0f;

    box.w *= scale;
    box.h *= scale;

    box.x = centerX - box.w / 2.0f;
    box.y = centerY - box.h / 2.0f;

    box.x = std::clamp(box.x, 0.0f, 1.0f);
    box.y = std::clamp(box.y, 0.0f, 1.0f);
    box.w = std::min(box.w, 1.0f - box.x);
    box.h = std::min(box.h, 1.0f - box.y);

    return box;
}