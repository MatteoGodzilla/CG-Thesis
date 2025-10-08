#include "ui.h"

void UI::begin(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
}

void UI::settings(){
    ImGui::Begin("Settings");
    ImGui::Checkbox("Continuous rendering", &active.alwaysDispatch);
    if(ImGui::Button("Render") || active.alwaysDispatch){
        dispatch.set();
    }
    if(ImGui::Button("Load compute shader")){
        loadCompute.set();
    }
    if(ImGui::Button("Export to png")) {
        exportImage.set();
    }
    ImGui::SeparatorText("Render settings");
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
    ImGui::SeparatorText("Viewport settings");
    const char* filterLabels[] = {"Pixel perfect", "Fill", "Stretch"};
    ImGui::Combo("Filter", (int*)&(active.filter), filterLabels, 3);

    ImGui::End();
}

void UI::universe(Camera *camera, Background* background, std::vector<Planet>* ref){
    ImGuiWindowFlags flag = 0;
    if(dirtyUniverse.getState()){
        flag |= ImGuiWindowFlags_UnsavedDocument;
    }
    ImGui::Begin("Universe", NULL, flag);
    if(ImGui::Button("Load")){
        loadUniverse.set(); 
    }
    ImGui::SameLine();
    if(ImGui::Button("Save")){
        saveUniverse.set();
        dirtyUniverse.clear();
    }
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
    ImGui::SeparatorText("Edit");
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

    ImVec2 availableSpace = ImGui::GetContentRegionAvail();
    active.viewportSize[0] = availableSpace.x;
    active.viewportSize[1] = availableSpace.y;

    ImVec2 mousePixel = mouseToComputePixel();
    ImGui::Image((ImTextureID)framebufferTexture, availableSpace);
    if(ImGui::IsItemHovered() && mousePixel.x >= 0 && mousePixel.y >= 0 && mousePixel.x < active.resolution[0] && mousePixel.y < active.resolution[1]) {
        float pixel[4] = {0};
        glGetTextureSubImage(computeTexture, 0, mousePixel.x, mousePixel.y, 0, 1, 1, 1, GL_RGBA, GL_FLOAT, 4 * sizeof(GL_FLOAT), &pixel); 
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

ImVec2 UI::mouseToComputePixel(){
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse = io.MousePos;

    ImVec2 imagePos = ImGui::GetCursorScreenPos();
    ImVec2 availableSpace = ImVec2(active.viewportSize[0], active.viewportSize[1]);

    switch(active.filter){
        case VF_PIXEL_PERFECT:
            return ImVec2(
                mouse.x - (imagePos.x + availableSpace.x / 2) + active.resolution[0] / 2,
                mouse.y - (imagePos.y + availableSpace.y / 2) + active.resolution[1] / 2
            );
        case VF_FILL: {
            float outputRatio = float(active.resolution[0]) / active.resolution[1];
            float width = std::min(availableSpace.x, availableSpace.y * outputRatio);    
            float height = std::min(availableSpace.y, availableSpace.x / outputRatio);    

            float widthRemaining = availableSpace.x - width;
            float heightRemaining = availableSpace.y - height;

            return ImVec2(
                (mouse.x - imagePos.x - widthRemaining / 2) * active.resolution[0] / width,
                (mouse.y - imagePos.y - heightRemaining / 2) * active.resolution[1] / height 
            );
        }
        case VF_STRETCH:
            return ImVec2(
                (mouse.x - imagePos.x) * active.resolution[0] / availableSpace.x,
                (mouse.y - imagePos.y) * active.resolution[1] / availableSpace.y
            );
        default:
            return ImVec2(0,0);
    }
}

Settings* UI::getSettings(){
    return &active;
}

