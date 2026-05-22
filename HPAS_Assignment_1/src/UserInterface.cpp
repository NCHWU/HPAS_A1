#include "UserInterface.h"

UserInterface::UserInterface(GLFWwindow* window, float dpiScale)
{
    ImGui::CreateContext();
    // Setup Dear ImGui style.
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    defaultFont = io.Fonts->AddFontFromFileTTF(RESOURCE_ROOT "framework/fonts/RobotoMono-Medium.ttf", 16.0f * dpiScale);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
}

UserInterface::~UserInterface()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UserInterface::start()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::PushFont(defaultFont);

    ImGui::Begin("User Interface");
}

void UserInterface::end()
{
    ImGui::PopFont();
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
