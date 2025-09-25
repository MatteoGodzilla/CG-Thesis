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
    ImGui::Checkbox("Continuous dispatch", &active.alwaysDispatch);
    if(ImGui::Button("Render") || active.alwaysDispatch){
        shouldDispatch = true;
    }

    ImGui::End();
}

void UI::universe(std::vector<Planet>* ref){
    ImGui::Begin("Universe");
    for(size_t i = 0; i < ref->size(); i++){
        Planet& p = ref->at(i);
        if(ImGui::TreeNode(p.name.c_str())){
            float posArray[3] = {p.position.x, p.position.y, p.position.z};
            float colorArray[3] = {p.color.x, p.color.y, p.color.z};
            bool modified = false;
            modified |= ImGui::DragFloat3("Position", posArray);
            modified |= ImGui::ColorEdit3("Color", colorArray);
            modified |= ImGui::InputFloat("Radius (m)", &(p.radius));
            modified |= ImGui::InputFloat("Mass (kg)", &(p.mass));
            p.position.x = posArray[0];
            p.position.y = posArray[1];
            p.position.z = posArray[2];
            p.color.x = colorArray[0];
            p.color.y = colorArray[1];
            p.color.z = colorArray[2];

            shouldUpdateUniverse |= modified;

            ImGui::TreePop();
        }
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

bool UI::shouldUpdateUniverseFlag(){
    return shouldUpdateUniverse;
}

void UI::clearUpdateUniverseFlag(){
    shouldUpdateUniverse = 0;
}
