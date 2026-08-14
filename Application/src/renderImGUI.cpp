#include <cstdlib>

#include "model.hpp"
#include "renderImGUI.hpp"
#include "renderSDL.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"


void RenderImGUI::setRenderer(RenderSDL* renderer) {
    sdlRenderer = renderer;
}

void RenderImGUI::render(Model& model) {
    // Begin ImGUI frame
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImVec2 displaySize = ImGui::GetMainViewport()->Size;

    float panelY = cameraBottom + gap;
    float panelHeight = displaySize.y - panelY;

    ImGui::SetNextWindowPos(ImVec2(0, panelY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    ImGui::Text("Circles: %d", (int)model.getCircles().size());

    // Add circle with random properties
    if (ImGui::Button("+", ImVec2(buttonWidth, 0))) {
        float minX = arenaWidth + spawnPadding;
        float maxX = displaySize.x - spawnPadding;

        float x = minX + rand() % (int)(maxX - minX);
        float y = spawnPadding + rand() % (int)(displaySize.y - spawnPadding * 2);

        float radius = minRadius + rand() % (int)(maxRadius - minRadius);

        float r = (rand() % 100) / 100.0f;
        float g = (rand() % 100) / 100.0f;
        float b = (rand() % 100) / 100.0f;

        model.addCircle(x, y, radius, r, g, b);
    }

    ImGui::SameLine();

    // Remove last circle added
    if (ImGui::Button("-", ImVec2(buttonWidth, 0)))
        model.removeCircle();
    
    // Toggle tracking and display features
    float wideButton = buttonWidth * 2 + gap;

    const char* cameraLabel = sdlRenderer->isCameraEnabled() ? "Camera: On" : "Camera: Off";
    if (ImGui::Button(cameraLabel, ImVec2(wideButton, 0)))
        sdlRenderer->toggleCamera();

    const char* handLabel = sdlRenderer->isHandTrackingEnabled() ? "Hand Track: On" : "Hand Track: Off";
    if (ImGui::Button(handLabel, ImVec2(wideButton, 0)))
        sdlRenderer->toggleHandTracking();

    const char* jointsLabel = sdlRenderer->isDisplayJointsEnabled() ? "Joints: On" : "Joints: Off";
    if (ImGui::Button(jointsLabel, ImVec2(wideButton, 0)))
        sdlRenderer->toggleDisplayJoints();

    const char* bboxLabel = sdlRenderer->isDisplayBBoxEnabled() ? "BBox: On" : "BBox: Off";
    if (ImGui::Button(bboxLabel, ImVec2(wideButton, 0)))
        sdlRenderer->toggleDisplayBBox();

    ImGui::End();
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdlRenderer->getRenderer());
}