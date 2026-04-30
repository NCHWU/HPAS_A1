#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <framework/shaders/Shader.h>
#include <framework/Camera.h>
#include <framework/Text.h>

#include <framework/ScrollManager.h>
#include <framework/NDArray.h>

#include <sstream>

#include "../../src/Settings.h"

#include "../../src/tsne/tsne.h"
#include <framework/DebugRenderData.h>

// Returns a string with the number a_value with n decimal points
// https://stackoverflow.com/questions/16605967/set-precision-of-stdto-string-when-converting-floating-point-values
template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return std::move(out).str();
}

// A grid line in the plot
// The "world height" is the distance from the origin along the given axis in world space
// The value is the value of the grid line in the graph
struct GridLine
{
	float worldHeight;
	float value;
};

namespace PlotRenderer
{
	// Initialize the plot renderer: does not need to be called by the student
	void init();

	// Draw a 2D scatter plot
	void scatterPlot(Camera& camera, NDArray<vec> points, Settings& settings, DebugRenderData2D debugRenderData = DebugRenderData2D());

	// Draw a 2D scatter plot with labels
	void scatterPlot(Camera& camera, NDArray<vec> points, NDArray<int>& labels, Settings& settings, DebugRenderData2D debugRenderData = DebugRenderData2D());
}