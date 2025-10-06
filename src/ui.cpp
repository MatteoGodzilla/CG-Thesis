#include "ui.h"

void UI::begin(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
}

void UI::settings(){
    ImGui::Begin("Settings");
    
    ImGui::DragInt2("Resolution", dirtyResolution);
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
        dispatch.set();
    }
    if(ImGui::Button("Save to file")) {
        exportImage.set();
    }

    ImGui::End();
}

void UI::universe(Camera *camera, Background* background, std::vector<Planet>* ref){
    ImGuiWindowFlags flag = 0;
    if(dirtyUniverse.getState()){
        flag |= ImGuiWindowFlags_UnsavedDocument;
    }
    ImGui::Begin("Universe", NULL, flag);
    ImGui::InputText("Name", newName, BUF_SIZE);
    ImGui::SameLine();
    if(ImGui::Button("Add")){
        ref->push_back({
            .name = std::string(newName, BUF_SIZE),
            .position = glm::vec3(0,0,0),
            .color = glm::vec3(0,0,0),
            .radius = 1,
            .mass = 1
        });
        dirtyUniverse.set();
    }
    if(ImGui::Button("Save to universe.txt")){
        saveUniverse.set();
        dirtyUniverse.clear();
    }
    ImGui::Separator();
    if(ImGui::TreeNode("Camera")){
        float posArray[3] = {camera->position.x, camera->position.y, camera->position.z};
        float lookArray[3] = {camera->look.x, camera->look.y, camera->look.z};
        float upArray[3] = {camera->up.x, camera->up.y, camera->up.z};
        bool modified = false;
        modified |= ImGui::DragFloat3("Position (m)",posArray, 1.0f, 0.0f, 0.0f, "%.3e");
        modified |= ImGui::DragFloat3("Look",lookArray);
        modified |= ImGui::DragFloat3("Up",upArray);
        modified |= ImGui::DragFloat("Vertical FOV (m)", &(camera->verticalFOV), 1.0f, 0.0f, 0.0f, "%.3e");
        if(modified){
            camera->position = glm::vec3(posArray[0], posArray[1], posArray[2]);
            camera->look = glm::vec3(lookArray[0], lookArray[1], lookArray[2]);
            camera->up = glm::vec3(upArray[0], upArray[1], upArray[2]);
            dirtyUniverse.set();
        }

        ImGui::TreePop();
    }
    if(ImGui::TreeNode("Background")){
        bool modified = false;
        float gridArray[2] = {background->gridSize.x, background->gridSize.y};
        modified |= ImGui::DragFloat2("Grid Size (m)", gridArray, 1.0f, 0.0f, 0.0f, "%.3e");
        modified |= ImGui::DragFloat("Distance (m)", &(background->distance), 1.0f, 0.0f, 0.0f, "%.3e");
        if(modified){
            background->gridSize = glm::vec2(gridArray[0], gridArray[1]);
            dirtyUniverse.set();
        }
        ImGui::TreePop();
    }
    for(size_t i = 0; i < ref->size(); i++){
        Planet& p = ref->at(i);
        if(ImGui::TreeNode(p.name.c_str())){
            float posArray[3] = {p.position.x, p.position.y, p.position.z};
            float colorArray[3] = {p.color.x, p.color.y, p.color.z};
            bool modified = false;
            modified |= ImGui::DragFloat3("Position (m)", posArray, 1.0f, 0.0f, 0.0f, "%.3e");
            modified |= ImGui::ColorEdit3("Color", colorArray);
            modified |= ImGui::InputFloat("Radius (m)", &(p.radius), 1.0f, 0.0f, "%.3e");
            modified |= ImGui::InputFloat("Mass (kg)", &(p.mass), 1.0f, 0.0f, "%.3e");

            if(ImGui::Button("Remove")){
                ref->erase(ref->begin() + i);
                dirtyUniverse.set();
            }

            if(modified){
                p.position = glm::vec3(posArray[0], posArray[1], posArray[2]);
                p.color = glm::vec3(colorArray[0], colorArray[1], colorArray[2]);
                updateUniverse.set();
                dirtyUniverse.set();
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void UI::viewport(GLuint framebufferTexture, GLuint computeTexture, std::vector<Planet>* ref){
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin("ViewPort");

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse = io.MousePos;

    ImVec2 availableSpace = ImGui::GetContentRegionAvail();
    active.viewportSize[0] = availableSpace.x;
    active.viewportSize[1] = availableSpace.y;

    ImVec2 imagePos = ImGui::GetCursorScreenPos();
    ImVec2 offset = ImVec2(mouse.x - imagePos.x, mouse.y - imagePos.y);

    ImGui::Image((ImTextureID)framebufferTexture, availableSpace);
    if(ImGui::IsItemHovered() && offset.x >= 0 && offset.y >= 0 && offset.x < active.resolution[0] && offset.y < active.resolution[1]) {
        float pixel[4] = {0};
        glGetTextureSubImage(computeTexture, 0, offset.x, offset.y, 0, 1, 1, 1, GL_RGBA, GL_FLOAT, 4 * sizeof(GL_FLOAT), &pixel); 
        int planetIndex = static_cast<int>(pixel[0]);
        int bounces = static_cast<int>(pixel[1]);
        if(planetIndex >= 0){
            ImGui::BeginTooltip();
            ImGui::Text(" %s\n Bounces: %d", ref->at(planetIndex).name.c_str(), bounces);
            ImGui::EndTooltip();
        }
    }

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

