#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <framework/NDArray.h>

#define START_TIMER() double time = glfwGetTime();
#define RESET_TIMER() time = glfwGetTime();
#define LOG_TIME(msg) \
    std::cout << msg << ": " << (glfwGetTime() - time) << " s" << std::endl; \
    time = glfwGetTime();

namespace PCA
{
	std::vector<glm::dvec2> embed(NDArray<float>& data);
}
