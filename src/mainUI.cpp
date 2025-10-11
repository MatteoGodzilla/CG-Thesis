#include "mainUI.h"

void GLAPIENTRY MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
    if(type == GL_DEBUG_TYPE_ERROR){
        std::cerr << "GL ERROR: " << message << std::endl; 
    } else {
        std::cout << "LOG: " << message << std::endl;
    }
}

int mainUI(std::istream& input, std::string lastOpenedFile){
    /* Initialize the library */
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CG-Thesis", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return 2;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        glfwTerminate();
        return 3;
    }
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);

    std::cout << vendor << std::endl;
    std::cout << renderer << std::endl;

    //Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    if (!std::filesystem::exists("imgui.ini")) {
        ImGui::LoadIniSettingsFromDisk("imgui_default.ini");
    }

    UI ui;
    Raytracer raytracer("compute.shader");
    Settings* defaultSettings = ui.getSettings();
    raytracer.update(defaultSettings->resolution[0], defaultSettings->resolution[1]);

    Viewport viewport;

    //Buffer
    GLuint transmissionBuffer;
    glGenBuffers(1, &transmissionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, transmissionBuffer);

    std::vector<Planet> planets;
    deserializeAll(input, &(raytracer.camera), &(raytracer.background), &planets);

    std::vector<PlanetGLSL> converted = planetsToGLSL(&planets);
    glBufferData(GL_SHADER_STORAGE_BUFFER, converted.size() * sizeof(PlanetGLSL), converted.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, transmissionBuffer);

    //Framebuffer
    Framebuffer framebuffer;
    framebuffer.update(WINDOW_WIDTH, WINDOW_HEIGHT);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        //---Renderer---
        Settings* settings = ui.getSettings();
        int w = settings->resolution[0];
        int h = settings->resolution[1];
        if(ui.dispatch.getState() || (settings->alwaysDispatch && ui.outdatedRender.getState())){
            raytracer.update(w, h);
            raytracer.dispatch(w, h);
            ui.dispatch.clear();
            ui.outdatedRender.clear();
        }
        
        if(ui.loadUniverse.getState()){
            const char* f = tinyfd_openFileDialog("Load universe file", UNIVERSE, 0, nullptr, "text file", 0);
            std::cout << "Trying to open file :" << f << std::endl;
            if(f != nullptr){
                std::ifstream newInput(f);
                if(newInput.is_open()){
                    planets.clear();
                    deserializeAll(newInput, &(raytracer.camera), &(raytracer.background), &planets);
                    ui.updateUniverse.set();
                    lastOpenedFile = std::string(f);
                    newInput.close();
                }
            }
            ui.loadUniverse.clear();
        }

        if(ui.updateUniverse.getState()){
            std::vector<PlanetGLSL> converted = planetsToGLSL(&planets);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, transmissionBuffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, converted.size() * sizeof(PlanetGLSL), converted.data(), GL_STATIC_READ);
            ui.updateUniverse.clear();
        }

        if(ui.saveUniverse.getState()){
            std::ofstream output(lastOpenedFile);
            if(output.is_open()){
                serializeAll(output, &(raytracer.camera), &(raytracer.background), &planets);
                output.close();
            }
            ui.saveUniverse.clear();
            ui.dirtyUniverse.clear();
        }

        if(ui.saveUniverseAs.getState()){
            const char* f = tinyfd_saveFileDialog("Save universe", UNIVERSE, 0, nullptr,nullptr);
            if(f != nullptr){
                std::ofstream output(f);
                if(output.is_open()){
                    serializeAll(output, &(raytracer.camera), &(raytracer.background), &planets);
                    lastOpenedFile = std::string(f);
                    output.close();
                }
            }
            ui.saveUniverseAs.clear();
            ui.dirtyUniverse.clear();
        }
    
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        if(ui.exportImage.getState()){
            //Get string of file to save to (autogenerated)
            std::ostringstream filename;
            auto t = std::time(nullptr);
            filename << std::put_time(std::localtime(&t), "output_%d-%m-%Y_%H-%M-%S.png");
            const char* f = tinyfd_saveFileDialog("Save rendered image", filename.str().c_str(), 0, nullptr,nullptr);
            if(f != nullptr){
                //Get texture data
                std::vector<unsigned char> output (w * h * 3); //rgb
                glBindTexture(GL_TEXTURE_2D, raytracer.getOutputTexture());
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &output[0]);
                stbi_write_png(f, w, h, 3, output.data(), w * 3);
                std::cout << "Saved to " << f << std::endl;
            }
            ui.exportImage.clear();
        }

        if(ui.loadCompute.getState()){
            const char* f = tinyfd_openFileDialog("Load compute shader", "compute.shader", 0, nullptr, "text file", 0);
            if(f != nullptr){
                raytracer.changeProgram(f);
            }
            ui.loadCompute.clear();
        }

        //---From compute shader output to framebuffer---
        framebuffer.bind();
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        viewport.update(settings, raytracer.getOutputTexture());
        viewport.draw(settings->filter);
        framebuffer.unbind();
      
        //---UI---
        ui.begin();
        ImGui::ShowDemoWindow(); 
        ui.settings();
        ui.universe(&(raytracer.camera), &(raytracer.background), &planets);
        ui.viewport(framebuffer.getColorTexture(), raytracer.getDebugTexture(), &planets);
        ui.end();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
