// Disable compiler warnings in third-party code (which we cannot change).
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
// Include glad before glfw3.
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

#include <iostream>
#include "Callbacks.h"

namespace WindowHandler
{
	bool initialise(unsigned int width, unsigned int height, const char* title, int openglVersionMinor);

	void setNewWindowSize(unsigned int width, unsigned int height);

	GLFWwindow* getWindow();

	glm::ivec2 getWindowSize();
}