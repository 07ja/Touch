#include <cmath>
#include <cfloat>
#include <chrono>
#include <algorithm>

#include "model.hpp"


Model::Model(): handTracker("../Model/hand_detector.onnx", "../Model/joint_mapping.onnx") {
    handTracker.start();
}

Model::~Model() {
    handTracker.stop();
}

std::array<Joint, 21> Model::getJoints() const {
    return handTracker.getJoints();
}

std::vector<Circle>& Model::getCircles() {
    return circles;
}

BBox Model::getBBox() const {
    return handTracker.getBBox();
}

Gesture Model::getGesture() const {
    return gesture;
}

int Model::getHandSelected() const {
    return handSelected;
}

int Model::getHandGrabbed() const {
    return handGrabbed;
}

float Model::getHandX() const {
    return handX;
}

float Model::getHandY() const {
    return handY;
}

void Model::setVelocity(int index, float vx, float vy) {
    if (index < 0 || index >= (int)circles.size()) 
        return;

    circles[index].vx = vx;
    circles[index].vy = vy;
}

void Model::addCircle(float x, float y, float radius, float r, float g, float b) {
    // Create circles
    Circle c;
    c.x = x;
    c.y = y;
    c.radius = radius;
    c.r = r;
    c.g = g;
    c.b = b;
    c.mass = radius * radius;
    c.vx = 0.0f;
    c.vy = 0.0f;
    circles.push_back(c);
}

void Model::removeCircle() {
    if (!circles.empty())
        circles.pop_back();
}

void Model::moveCircle(int index, float x, float y, int windowWidth, int windowHeight, float arenaX) {
    if (index < 0 || index >= (int)circles.size()) 
        return;

    Circle& c = circles[index];

    // Keep circle inside arena
    if (x - c.radius < arenaX)
        x = arenaX + c.radius;
    if (x + c.radius > windowWidth)
        x = windowWidth  - c.radius;
    if (y - c.radius < 0)
        y = c.radius;
    if (y + c.radius > windowHeight)
        y = windowHeight - c.radius;

    c.x = x;
    c.y = y;
}

void Model::updatePhysics(float dt, int width, int height, float arenaWidth) {
    // Update circle physics
    for (int i = 0; i < (int)circles.size(); i++) {
        if (i == handGrabbed)
            continue;

        Circle& c = circles[i];

        // Update position and velocity
        c.x += c.vx * dt;
        c.y += c.vy * dt;

        c.vx *= friction;
        c.vy *= friction;

        keepCircleInside(c, width, height, arenaWidth);
    }
}

void Model::collisions(int width, int height, float arenaWidth) {
    // Resolve circle collisions
    for (int iter = 0; iter < collisionIterations; iter++) {
        for (int i = 0; i < circles.size(); i++) {
            for (int j = i + 1; j < circles.size(); j++) {
                float nx;
                float ny;
                float overlap;
                
                if (checkCollision(circles[i], circles[j], nx, ny, overlap)) {
                    resolvePosition(circles[i], circles[j], nx, ny, overlap, i,j);
                    resolveVelocity(circles[i], circles[j], nx, ny, i,j);
                }
            }
        }

        for (auto& c : circles) {
            keepCircleInside(c, width, height, arenaWidth);
        }
    }
}

void Model::updateHandTracking(SDL_Surface* cameraFrame) {
    // Submit camera frame
    handTracker.submitFrame(cameraFrame);
}

void Model::updateGesture(int width, int height) {
    if (!isHandDetected()) {
        gesture = Gesture::None;
        handSelected = -1;
        return;
    }

    // Update hand state
    auto joints = handTracker.getJoints();
    previousGesture = gesture;
    gesture = detectGesture(joints);

    updateHandCursor(joints, width, height);
    updateHandVelocity();
    updateSelection();

    // Handle grab and release
    if (gesture == Gesture::Fist && previousGesture != Gesture::Fist)
        grabCircle();

    if (gesture == Gesture::Fist)
        moveGrabbedCircle();

    if (gesture != Gesture::Fist && previousGesture == Gesture::Fist)
        releaseCircle();
}

bool Model::isHandDetected() const {
    return handTracker.isHandDetected();
}


// Private Helper Functions

bool Model::isFingerExtended(const std::array<Joint, 21>& j, int tip, int base) const {
    // Check finger extension 
    return j[tip].y < j[base].y - fingerThreshold;
}

Gesture Model::detectGesture(const std::array<Joint, 21>& j) const {
    bool indexUp = isFingerExtended(j, 8,  6);
    bool middleUp = isFingerExtended(j, 12, 10);
    bool ringUp = isFingerExtended(j, 16, 14);
    bool pinkyUp = isFingerExtended(j, 20, 18);

    // Identify hand gesture performed
    if (!indexUp && !middleUp && !ringUp && !pinkyUp)
        return Gesture::Fist;

    if (indexUp && middleUp && !ringUp && !pinkyUp)
        return Gesture::TwoFingerPoint;

    return Gesture::None;
}

void Model::updateHandCursor(const std::array<Joint, 21>& joints, int width, int height) {
    // Compute frame time
    auto now = std::chrono::steady_clock::now();
    float dt = firstFrame ? 1.0f / 60.0f : std::chrono::duration<float>(now - lastFrameTime).count();

    lastFrameTime = now;
    firstFrame = false;

    // Scale hand position
    float rawX = 1.0f - joints[8].x;
    float rawY = joints[8].y;

    float offset = (cursorSensitivity - 1.0f) / 2.0f;

    float scaledX = std::clamp(rawX * cursorSensitivity - offset, 0.0f, 1.0f);
    float scaledY = std::clamp(rawY * cursorSensitivity - offset, 0.0f, 1.0f);

    prevHandX = handX;
    prevHandY = handY;

    // Smooth cursor movement
    handX = handFilterX.filter(scaledX * width, dt);
    handY = handFilterY.filter(scaledY * height, dt);
}

void Model::updateHandVelocity() {
    // Update hand velocity
    float dx = handX - prevHandX;
    float dy = handY - prevHandY;

    handVx = handVx * cursorVelocityDecay + dx * cursorVelocitySmoothing;
    handVy = handVy * cursorVelocityDecay + dy * cursorVelocitySmoothing;
}

void Model::updateSelection() {
    // Update selection timer
    if (handGraceFrames > 0) {
        handGraceFrames--;

        if (handGraceFrames == 0)
            lastHandSelected = -1;
    }

    // Check for pointing gesture
    if (gesture != Gesture::TwoFingerPoint) {
        handSelected = -1;
        return;
    }

    float bestDistance = FLT_MAX;
    handSelected = -1;

    // Find the nearest circle
    for (int i = 0; i < (int)circles.size(); i++) {
        float dx = handX - circles[i].x;
        float dy = handY - circles[i].y;

        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance < circles[i].radius * selectRadius && distance < bestDistance) {
            bestDistance = distance;
            handSelected = i;
        }
    }

    // Store selected circle
    if (handSelected != -1) {
        lastHandSelected = handSelected;
        handGraceFrames = graceLimit;
    }
}

void Model::grabCircle() {
    // Select circle to grab
    int index = handSelected;

    if (index == -1)
        index = lastHandSelected;

    if (index == -1)
        return;

    handGrabbed = index;

    // Reset velocities
    circles[index].vx = 0.0f;
    circles[index].vy = 0.0f;

    handVx = 0.0f;
    handVy = 0.0f;

    lastHandSelected = -1;
    handGraceFrames = 0;
}

void Model::moveGrabbedCircle() {
    if (handGrabbed < 0 || handGrabbed >= circles.size())
        return;

    Circle& c = circles[handGrabbed];

    // Move the grabbed circle
    c.x = handX;
    c.y = handY;

    c.vx = 0;
    c.vy = 0;
}

void Model::releaseCircle() {
    if (handGrabbed < 0 || handGrabbed >= (int)circles.size())
        return;

    // Apply throw velocity
    circles[handGrabbed].vx = handVx * throwScale;
    circles[handGrabbed].vy = handVy * throwScale;

    handGrabbed = -1;

    handVx = 0.0f;
    handVy = 0.0f;
}

void Model::keepCircleInside(Circle& c, int width, int height, float arenaWidth) {
    // Keep circle inside bounds
    if (c.x - c.radius < arenaWidth) {
        c.x = arenaWidth + c.radius;
        c.vx *= -bounce;
    }

    if (c.x + c.radius > width) {
        c.x = width - c.radius;
        c.vx *= -bounce;
    }

    if (c.y - c.radius < 0) {
        c.y = c.radius;
        c.vy *= -bounce;
    }

    if (c.y + c.radius > height) {
        c.y = height - c.radius;
        c.vy *= -bounce;
    }
}

bool Model::checkCollision(Circle& a, Circle& b, float& nx, float& ny, float& overlap) {
    // Check if circles overlap
    float dx = b.x - a.x;
    float dy = b.y - a.y;

    float distance = std::sqrt(dx * dx + dy * dy);
    float minDistance = a.radius + b.radius;

    if (distance >= minDistance)
        return false;

    if (distance < minimumDistance) {
        dx = minimumDistance;
        dy = 0.0f;
        distance = minimumDistance;
    }

    nx = dx / distance;
    ny = dy / distance;

    overlap = (minDistance - distance) / 2.0f;

    return true;
}

void Model::resolvePosition(Circle& a, Circle& b, float nx, float ny, float overlap, int indexA, int indexB) {
    // Separate circles which overlap
    bool aGrabbed = (indexA == handGrabbed);
    bool bGrabbed = (indexB == handGrabbed);

    if (!aGrabbed) {
        a.x -= nx * overlap;
        a.y -= ny * overlap;
    }

    if (!bGrabbed) {
        b.x += nx * overlap;
        b.y += ny * overlap;
    }
}

void Model::resolveVelocity(Circle& a, Circle& b, float nx, float ny, int indexA, int indexB) {
    // Compute collision between circles
    bool aGrabbed = (indexA == handGrabbed);
    bool bGrabbed = (indexB == handGrabbed);

    float relativeX = b.vx - a.vx;
    float relativeY = b.vy - a.vy;

    float velocity = relativeX * nx + relativeY * ny;

    if (velocity > 0)
        return;

    // Apply collision impulse and update circles
    float impulse = -(1 + restitution) * velocity / (1.0f / a.mass + 1.0f / b.mass);

    float ix = impulse * nx;
    float iy = impulse * ny;

    if (!aGrabbed) {
        a.vx -= ix / a.mass;
        a.vy -= iy / a.mass;
    }

    if (!bGrabbed) {
        b.vx += ix / b.mass;
        b.vy += iy / b.mass;
    }
}