#pragma once

#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "framework/FileDialogs.h"
#include "framework/SavedSettings.h"

class UserInterface
{
public:
	UserInterface(GLFWwindow* window, float dpiScale);
	~UserInterface();

	void start();

	void end();

private:
	ImFont* defaultFont{ nullptr };
};