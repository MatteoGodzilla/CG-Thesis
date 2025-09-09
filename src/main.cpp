#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <vector>

void GLAPIENTRY
MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
    if(type == GL_DEBUG_TYPE_ERROR){
        std::cerr << "GL ERROR: " << message << std::endl; 
    }
}

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(){
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tesi OpenGL", NULL, NULL);
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

    std::ifstream computeFile = std::ifstream("compute.shader");
    if(!computeFile.is_open()){
        glfwTerminate();
        return 4;
    }

    //Read shader
    std::string computeSource = "";
    std::string temp = "";
    while(std::getline(computeFile, temp)){
        computeSource += temp + "\n";
    }
    const char* computeSourceRaw = computeSource.c_str();
    std::cout << computeSourceRaw << std::endl; 
    
    GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &computeSourceRaw , NULL); 
    glCompileShader(compute);

    GLuint rayTracer = glCreateProgram();
    glAttachShader(rayTracer, compute);
    glLinkProgram(rayTracer);
    
    //Texture buffer 
    //TODO: make this dynamic based on viewport dimentions
    int textureWidth = WINDOW_WIDTH;
    int textureHeight = WINDOW_HEIGHT;
    GLuint textureOutput;
    glGenTextures(1, &textureOutput);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, textureOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    //Get maximum work group count
    //TODO: calculate best amount of work groups
    int workGroupMax[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupMax[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupMax[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupMax[2]);

    //std::cout << workGroupMax[0] << ", " << workGroupMax[0] << ", " << workGroupMax[0] << ", " << std::endl;

    //Vertex
    std::ifstream vertexFile = std::ifstream("vertex.shader");
    if(!vertexFile.is_open()){
        glfwTerminate();
        return 5;
    }
    std::string vertexSource = "";
    while(std::getline(vertexFile, temp)){
        vertexSource += temp + "\n";
    }
    std::cout << "========================" << std::endl;
    std::cout << vertexSource << std::endl;

    //Fragment
    std::ifstream fragmentFile = std::ifstream("fragment.shader");
    if(!fragmentFile.is_open()){
        glfwTerminate();
        return 6;
    }
    std::string fragmentSource = "";
    while(std::getline(fragmentFile, temp)){
        fragmentSource += temp + "\n";
    }
    std::cout << "========================" << std::endl;
    std::cout << fragmentSource << std::endl;
    
    //Display program
    GLuint display = glCreateProgram();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);    
    const char* vertexSourceRaw = vertexSource.c_str();
    const char* fragmentSourceRaw = fragmentSource.c_str();
    glShaderSource(vertexShader, 1, &vertexSourceRaw, NULL);
    glShaderSource(fragmentShader, 1, &fragmentSourceRaw, NULL);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
    glAttachShader(display, vertexShader);
    glAttachShader(display, fragmentShader);
    glLinkProgram(display);

    //VAO    
    GLuint quadVAO;
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);
    //Position
    GLuint quadPositionVBO;
    glGenBuffers(1, &quadPositionVBO);
    float positions[] = {
        -1, 1,
        1, 1, 
        1, -1,
        -1, -1
    };
    glBindBuffer(GL_ARRAY_BUFFER, quadPositionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    //Indicies
    GLuint quadIndiciesVBO;
    glGenBuffers(1, &quadIndiciesVBO);
    unsigned int indicies[] = {
        0, 1, 2,
        2, 3, 0
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndiciesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);
    //uvs
    GLuint quadTextureVBO;
    glGenBuffers(1, &quadTextureVBO);
    float uvs[] = {
        0.0, 1.0,
        1.0, 1.0,
        1.0, 0.0,
        0.0, 0.0
    };
    glBindBuffer(GL_ARRAY_BUFFER, quadTextureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);


    //Buffer
    GLuint transmissionBuffer;
    glGenBuffers(1, &transmissionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, transmissionBuffer);
    srand(time(NULL)); 
    std::vector<float> transmissionData;

    for(int i = 0; i < 5 * 3; i++){
        float v = (float)rand() / RAND_MAX;
        transmissionData.push_back(v);
        std::cout << i << ":" << v << std::endl; 
    }

    /*
    float transmissionData[] = {
        //x normalized, y normalized, r, g, b
        0.33, 0.33, 1.0, 1.0, 0.0,
        0.66, 0.66, 0.0, 1.0, 0.0
        //0.5, 0.5, 0, 0, 1,
    };
    */
    glBufferData(GL_SHADER_STORAGE_BUFFER, transmissionData.size() * sizeof(float), transmissionData.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, transmissionBuffer);


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Dispatch compute shader
        glUseProgram(rayTracer);
        //TODO: calculate best amount of work groups
        glDispatchCompute((GLuint)textureWidth, (GLuint)textureHeight, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        /* Render here */
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(display);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureOutput);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

