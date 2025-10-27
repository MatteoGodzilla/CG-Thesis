#include "mainRenderer.h"

int mainRenderer(int width, int height, std::istream& input, std::string output){
    //TODO: look into opengl context without window
    //for now we just create one hidden window just to resolve the function pointers
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

    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);

    std::cout << "Using " << renderer << " by " << vendor << std::endl;

    //Create raytracer
    Raytracer raytracer("shaders/compute.shader");
    Universe universe;
    deserializeAll(input, &universe);
    raytracer.update(width, height, &universe);

    //dispatch
    raytracer.dispatch();
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
