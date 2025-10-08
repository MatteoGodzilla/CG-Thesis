#include "mainRenderer.h"

int mainRenderer(int width, int height, std::istream& input, std::string output){
    //We cannot use opengl functions without a window
    /* Initialize the library */
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_VISIBLE, false);
    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(1, 1, "", NULL, NULL);
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

    //Create raytracer
    Raytracer raytracer("compute.shader");
    raytracer.update(width, height);
    //load universe
    GLuint transmissionBuffer;
    glGenBuffers(1, &transmissionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, transmissionBuffer);

    std::vector<Planet> planets;
    deserializeAll(input, &(raytracer.camera), &(raytracer.background), &planets);
    //Send planets to gpu
    std::vector<PlanetGLSL> converted = planetsToGLSL(&planets);
    glBufferData(GL_SHADER_STORAGE_BUFFER, converted.size() * sizeof(PlanetGLSL), converted.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, transmissionBuffer);

    //dispatch
    raytracer.dispatch(width, height);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    //save file
    std::vector<unsigned char> outputImage (width * height * 3); //rgb
    glBindTexture(GL_TEXTURE_2D, raytracer.getOutputTexture());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &outputImage[0]);
    stbi_write_png(output.c_str(), width, height, 3, outputImage.data(), width * 3);
    std::cout << "Saved to " << output << std::endl;
    glfwTerminate();
    return 0;
}
