#include "ui.h"

void UI::begin(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
}

void UI::settings(){
    ImGui::Begin("Settings");

    ImGui::DragInt2("Render resolution", dirtyResolution);
    if(ImGui::Button("Apply resolution")){
        active.resolution[0] = dirtyResolution[0];
        active.resolution[1] = dirtyResolution[1];
    }
    if(ImGui::Button("Set resolution to viewport")){
        active.resolution[0] = active.viewportSize[0];
        active.resolution[1] = active.viewportSize[1];
        dirtyResolution[0] = active.viewportSize[0];
        dirtyResolution[1] = active.viewportSize[1];
    }
    if(ImGui::Button("Render")){
        shouldDispatch = true;
    }

    ImGui::End();
}

void UI::viewport(GLuint framebufferTexture){
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin("ViewPort");

    ImVec2 availableSpace = ImGui::GetContentRegionAvail();
    active.viewportSize[0] = availableSpace.x;
    active.viewportSize[1] = availableSpace.y;

    ImGui::Image((ImTextureID)framebufferTexture, availableSpace);

    ImGui::End();
    ImGui::PopStyleVar();
}

void UI::end(){
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Settings* UI::getSettings(){
    return &active;
}

//---FLAGS---
bool UI::shouldDispatchFlag(){
    return shouldDispatch;
}

void UI::clearDispatchFlag(){
    shouldDispatch = 0;
}
