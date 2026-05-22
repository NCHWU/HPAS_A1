#include "WindowHandler.h"
namespace WindowHandler {
    namespace {
        GLFWwindow* window;
        std::string currentWindowName { "" };
        unsigned int windowWidth { 0 };
        unsigned int windowHeight { 0 };
    }

    bool initialise(unsigned int width, unsigned int height, const char* title, int openglVersionMinor)
    {
        windowWidth = width;
        windowHeight = height;

        // Initialising GLFW
        glfwInit();

        // Setting OpenGL version 4.x
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openglVersionMinor);

        // OpenGL profile: core
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        #ifdef __APPLE__
        // Required on macOS for a 4.1 core context
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif

        // Making a GLFW window
        window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (window == NULL) {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        glfwSetFramebufferSizeCallback(window, &Callbacks::framebuffer_size_callback);
        glfwSetScrollCallback(window, &Callbacks::scrollCallback);

        return true;
    }

    GLFWwindow* getWindow()
    {
        return window;
    }

    void setNewWindowSize(unsigned int width, unsigned int height)
    {
        windowWidth = width;
        windowHeight = height;
        glViewport(0, 0, width, height);
    }

    glm::ivec2 getWindowSize()
    {
        return glm::ivec2(windowWidth, windowHeight);
    }
}