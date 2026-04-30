#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Matrix math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "WindowHandler.h"
#include "ScrollManager.h"

class Callbacks
{
public:
    static Callbacks& getInstance();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void framebuffer_size_callbackImpl(GLFWwindow* window, int width, int height);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    void mouseCallbackImpl(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    void scrollCallbackImpl(GLFWwindow* window, double xoffset, double yoffset);
    static void characterCallback(GLFWwindow* window, unsigned int codepoint);
    void characterCallbackImpl(GLFWwindow* window, unsigned int codepoint);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void keyCallbackImpl(GLFWwindow* window, int key, int scancode, int action, int mods);

private:

    Callbacks(void)
    {
    }

    Callbacks(Callbacks const&);
    void operator=(Callbacks const&);
};