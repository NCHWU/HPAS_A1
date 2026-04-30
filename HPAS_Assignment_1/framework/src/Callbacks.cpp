#include "Callbacks.h"

Callbacks& Callbacks::getInstance()
{
    static Callbacks instance;
    return instance;
}

void Callbacks::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    getInstance().framebuffer_size_callbackImpl(window, width, height);
}

void Callbacks::framebuffer_size_callbackImpl(GLFWwindow* window, int width, int height)
{
    WindowHandler::setNewWindowSize(width, height);
}

void Callbacks::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    getInstance().mouseCallbackImpl(window, xpos, ypos);
}

void Callbacks::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    getInstance().scrollCallbackImpl(window, xoffset, yoffset);
}

void Callbacks::mouseCallbackImpl(GLFWwindow* window, double xpos, double ypos)
{
}

void Callbacks::scrollCallbackImpl(GLFWwindow* window, double xoffset, double yoffset)
{
	ScrollManager::onScroll(yoffset);
}

void Callbacks::characterCallback(GLFWwindow* window, unsigned int codepoint)
{
    getInstance().characterCallbackImpl(window, codepoint);
}

void Callbacks::characterCallbackImpl(GLFWwindow* window, unsigned int codepoint)
{
}

void Callbacks::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    getInstance().keyCallbackImpl(window, key, scancode, action, mods);
}

void Callbacks::keyCallbackImpl(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        //keyInput(key);
    }
}
