#include "ui.h"

void UI::begin(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
}

void UI::settings(){
    bool openModal = false;
    ImGui::BeginMainMenuBar();
    if(ImGui::BeginMenu("File")){
        if(ImGui::MenuItem("Load universe")){
            loadUniverse.set(); 
        }
        if(ImGui::MenuItem("Save universe")){
            saveUniverse.set();
        }
        if(ImGui::MenuItem("Save universe as")){
            saveUniverseAs.set();
        }
        if(ImGui::MenuItem("Load compute shader")){
            loadCompute.set();
        }
        if(ImGui::MenuItem("Export to png")) {
            exportImage.set();
        }
        ImGui::EndMenu();
    }
    std::string renderMenuLabel = "Render";
    if(outdatedRender.getState()){
        renderMenuLabel += " *";
    }
    if(ImGui::BeginMenu(renderMenuLabel.c_str())){
        std::string renderMenuItem = "Render frame";
        if(outdatedRender.getState()){
            renderMenuItem += " *";
        }
        if(ImGui::MenuItem(renderMenuItem.c_str())){
            dispatch.set();
        }
        ImGui::MenuItem("Continuous rendering", NULL, &alwaysDispatch);
        if(ImGui::MenuItem("Set custom resolution")){
            openModal = true;
        }
        if(ImGui::MenuItem("Set resolution to viewport")){
            resolution = viewportSize;
            dirtyResolution[0] = viewportSize.x;
            dirtyResolution[1] = viewportSize.y;
        }
        ImGui::EndMenu();
    }
    if(ImGui::BeginMenu("Viewport")){
        const char* filterLabels[] = {"Pixel perfect", "Fill", "Stretch"};
        for(int i = 0; i < 3; i++){
            bool isSelected = filter == i;
            if(ImGui::MenuItem(filterLabels[i], NULL, &isSelected)){
                filter = (ViewportFilter)i;
            }
        }
        ImGui::EndMenu(); 
    }
    if(ImGui::BeginMenu("Help")){
        ImGui::EndMenu(); 
    }

    if(openModal){
        ImGui::OpenPopup("Set custom resolution");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if(ImGui::BeginPopupModal("Set custom resolution", NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::DragInt2("Render resolution", dirtyResolution);
        if(ImGui::Button("Apply resolution")){
            resolution.x = dirtyResolution[0];
            resolution.y = dirtyResolution[1];
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel")){
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::EndMainMenuBar();
}

void UI::universe(Camera *camera, Background* background, std::vector<Planet>* ref){
    ImGuiWindowFlags flag = 0;
    if(dirtyUniverse.getState()){
        flag |= ImGuiWindowFlags_UnsavedDocument;
    }
    ImGui::Begin("Universe", NULL, flag);
    ImGui::SeparatorText("Add planet");
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
        outdatedRender.set();
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
            outdatedRender.set();
        }

        ImGui::TreePop();
    }
    if(ImGui::TreeNode("Background")){
        bool modified = false;
        const char* types[] = {"Solid", "Grid"};
        ImGui::Combo("Type", (int*)(&background->type), types, BG_SIZE);

        if(background->type == BG_SOLID){
            float colorATemp[] = {background->colorA.r, background->colorA.g, background->colorA.b};
            modified = ImGui::ColorEdit3("Color", colorATemp);
            if(modified){
                background->colorA = glm::vec3(colorATemp[0], colorATemp[1], colorATemp[2]);
            }
        } else if(background->type == BG_GRID){
            float colorATemp[] = {background->colorA.r, background->colorA.g, background->colorA.b};
            float colorBTemp[] = {background->colorB.r, background->colorB.g, background->colorB.b};
            float gridArray[2] = {background->gridSize.x, background->gridSize.y};
            modified |= ImGui::ColorEdit3("Color A", colorATemp);
            modified |= ImGui::ColorEdit3("Color B", colorBTemp);
            modified |= ImGui::DragFloat2("Grid Size (m)", gridArray, 1.0f, 0.0f, 0.0f, "%.3e");
            modified |= ImGui::DragFloat("Distance (m)", &(background->distance), 1.0f, 0.0f, 0.0f, "%.3e");
            if(modified){
                background->gridSize = glm::vec2(gridArray[0], gridArray[1]);
                background->colorA = glm::vec3(colorATemp[0], colorATemp[1], colorATemp[2]);
                background->colorB = glm::vec3(colorBTemp[0], colorBTemp[1], colorBTemp[2]);
            }
        }
        if(modified){
            dirtyUniverse.set();
            outdatedRender.set();
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
                outdatedRender.set();
            }

            if(modified){
                p.position = glm::vec3(posArray[0], posArray[1], posArray[2]);
                p.color = glm::vec3(colorArray[0], colorArray[1], colorArray[2]);
                updateUniverse.set();
                dirtyUniverse.set();
                outdatedRender.set();
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
    viewportSize.x = availableSpace.x;
    viewportSize.y = availableSpace.y;

    ImVec2 mouse = mouseToComputePixel();
    int x = std::floor(mouse.x);
    int y = std::floor(mouse.y);
    ImGui::Image((ImTextureID)framebufferTexture, availableSpace);
    if(ImGui::IsItemHovered() && x >= 0 && y >= 0 && x < resolution.x && y < resolution.y) {
        int width = resolution.x;
        float r = debugBuffer.at((x + y * width) * 4 + 0);
        float g = debugBuffer.at((x + y * width) * 4 + 1);
        float b = debugBuffer.at((x + y * width) * 4 + 2);
        float a = debugBuffer.at((x + y * width) * 4 + 3);
        ImGui::BeginTooltip();
        //ImGui::Text(" %s\n %d", ref->at(planetIndex).name.c_str(), bounces);
        ImGui::Text(" %f\n %f\n %f\n %f\n x:%d y:%d\n ", r, g, b, a, x, y);
        ImGui::EndTooltip();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void UI::copyDebugTexture(GLuint debugTexture){
    int width = resolution.x;
    int height = resolution.y;

    debugBuffer.clear();
    debugBuffer = std::vector<float>(width * height * 4);
    glBindTexture(GL_TEXTURE_2D, debugTexture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &debugBuffer[0]);
}

void UI::end(){
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

ImVec2 UI::mouseToComputePixel(){
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse = io.MousePos;

    ImVec2 imagePos = ImGui::GetCursorScreenPos();
    ImVec2 availableSpace = ImVec2(viewportSize.x, viewportSize.y);

    switch(filter){
        case VF_PIXEL_PERFECT:
            return ImVec2(
                mouse.x - (imagePos.x + availableSpace.x / 2) + resolution.x / 2,
                mouse.y - (imagePos.y + availableSpace.y / 2) + resolution.y / 2
            );
        case VF_FILL: {
            float outputRatio = float(resolution.x) / resolution.y;
            float width = std::min(availableSpace.x, availableSpace.y * outputRatio);    
            float height = std::min(availableSpace.y, availableSpace.x / outputRatio);    

            float widthRemaining = availableSpace.x - width;
            float heightRemaining = availableSpace.y - height;

            return ImVec2(
                (mouse.x - imagePos.x - widthRemaining / 2) * resolution.x / width,
                (mouse.y - imagePos.y - heightRemaining / 2) * resolution.y / height 
            );
        }
        case VF_STRETCH:
            return ImVec2(
                (mouse.x - imagePos.x) * resolution.x / availableSpace.x,
                (mouse.y - imagePos.y) * resolution.y / availableSpace.y
            );
        default:
            return ImVec2(0,0);
    }
}
