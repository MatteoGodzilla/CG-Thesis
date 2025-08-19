#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>

GLenum errorCheck();

void GLAPIENTRY
MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
    if(type == GL_DEBUG_TYPE_ERROR){
        std::cerr << "GL ERROR: " << message << std::endl; 
    }
}

int main(){
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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

    std::ifstream compute_file = std::ifstream("compute.shader");
    if(!compute_file.is_open())
        return 4;

    //Read shader
    std::string compute_source = "";
    std::string temp = "";
    while(std::getline(compute_file, temp)){
        compute_source += temp + "\n";
    }
    const char* compute_source_raw = compute_source.c_str();

    unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &compute_source_raw, NULL); 
    glCompileShader(compute);

    unsigned int program = glCreateProgram();
    glAttachShader(program, compute);
    glLinkProgram(program);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClearColor(1,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

