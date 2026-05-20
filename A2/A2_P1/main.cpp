// Disable compiler warnings in third-party code (which we cannot change).
#include <framework/disable_all_warnings.h>

#include "ft/ft.h"
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
// Include glad before glfw3.
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <numeric>
#include <span>
#include <string>
#include <vector>

#include "Settings.h"

#include "framework/Framework.h"
#include "framework/WindowHandler.h"
#include "framework/FileDialogs.h"
#include "framework/PlotRenderer.h"
#include "framework/Camera.h"
#include "UserInterface.h"
#include "tsne/tsne.h"
#include "tsne/GradientComputers/QuadtreeGradientCompute.h"
#include "tsne/GradientComputers/FItSNE.h"
#include "tsne/GradientComputers/WrappedFItSNE.h"
#include "tsne/square_distance/NaiveSquareDistanceCalculator.h"
#include "tsne/square_distance/GPUSquareDistanceCalculator.h"

#include "framework/datareaders/MyeloidReader.h"
#include "framework/datareaders/MNISTReader.h"
#include "framework/datareaders/PlanariaReader.h"

TSNEGradientCompute& getActiveGradientComputer(
    int activeGradientComputer,
    TSNEGradientCompute& exactGradientComputer,
    QuadtreeTSNE& quadtreeGradientComputer,
    FItSNE& fItSNEGradientComputer,
    WrappedFItSNE& wrappedFItSNEGradientComputer)
{
	switch (activeGradientComputer)
	{
		case 0:
			return exactGradientComputer;
		case 1:
			return quadtreeGradientComputer;
		case 2:
			return fItSNEGradientComputer;
		case 3:
			return wrappedFItSNEGradientComputer;
		default:
			throw std::invalid_argument("Invalid gradient computer index");
	}
}

void tooltip(const char* text)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::TextUnformatted(text);
		ImGui::EndTooltip();
	}
}

int main(int /* argc */, char** argv)
{
    unsigned int WINDOW_SIZE_X{ 1200 };
    unsigned int WINDOW_SIZE_Y{ 800 };

    if (!WindowHandler::initialise(WINDOW_SIZE_X, WINDOW_SIZE_Y, "High Performance Analysis Systems", 1))
    {
        // Something went wrong initialising the window
        return -1;
    }

    if (!Framework::initialise())
    {
        // Something went wrong initialising the framework
        return -1;
    }

	float xscale, yscale;
    glfwGetWindowContentScale(WindowHandler::getWindow(), &xscale, &yscale);
    UserInterface userInterface(WindowHandler::getWindow(), yscale);

    Camera camera;
    float lastTime{ 0.0f };

	bool runningTSNE{ false };

    TSNE tsne;

	NearestNeighbourCalculator::enable();

	TSNEGradientCompute exactGradientComputer = TSNEGradientCompute();
    QuadtreeTSNE quadtreeGradientComputer = QuadtreeTSNE();
    FItSNE fItSNEGradientComputer = FItSNE();
    WrappedFItSNE wrappedFItSNEGradientComputer = WrappedFItSNE();

    Settings settings;
    if (AUTO_LOAD_SETTINGS) settings = readSettingsFromFile();

    DebugRenderData debugRenderData;
	debugRenderData.enabled = false;

    // This implementation relies on the GPU
    GPUSquareDistanceCalculator squareDistanceCalculator;
    // If this one doesn't work, use the naive one:
	// NaiveSquareDistanceCalculator squareDistanceCalculator;


	int numberOfNeighboursConsidering = 10;

    while (true)
    {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = static_cast<float>(glfwGetTime());

        Settings savedSettings = settings;

        if (glfwWindowShouldClose(WindowHandler::getWindow()))
        {
            break;
        }

        // Clear the framebuffer with a white color
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.processInput(deltaTime);

		// If t-SNE has any points, render them
        if (tsne.hasPoints())
        {
            if (runningTSNE)
            {
                TSNEGradientCompute& activeGradientComputer = getActiveGradientComputer(
                    settings.activeGradientComputerIndex,
                    exactGradientComputer,
                    quadtreeGradientComputer,
                    fItSNEGradientComputer,
                    wrappedFItSNEGradientComputer);

                debugRenderData.clear();
                float tsneStepStartTime = static_cast<float>(glfwGetTime());
                tsne.step(activeGradientComputer, debugRenderData);
                float tsneStepEndTime = static_cast<float>(glfwGetTime());
				std::cout << "t-SNE step took " << tsneStepEndTime - tsneStepStartTime << " seconds" << std::endl;
				std::cout << "Running t-SNE at " << 1.0f / (tsneStepEndTime - tsneStepStartTime) << " steps / second" << std::endl;
            }
			PlotRenderer::scatterPlot(camera, tsne.getPoints(), tsne.getLabels(), settings, debugRenderData);
        }

		// All user interface rendering code should be
        // placed between userInterface.start() and userInterface.end()
        userInterface.start();

        ImGui::Text("Active gradient computer");
        ImGui::RadioButton("Exact", &settings.activeGradientComputerIndex, 0);
		ImGui::RadioButton("Quadtree", &settings.activeGradientComputerIndex, 1);
		ImGui::RadioButton("FItSNE", &settings.activeGradientComputerIndex, 2);
		ImGui::RadioButton("Wrapped FItSNE", &settings.activeGradientComputerIndex, 3);

        if (ImGui::BeginCombo("Load Dataset", "Select..."))
        {
            if (ImGui::Selectable("MNIST"))
            {
                MNISTReader mnistReader;
                mnistReader.readData(tsne, squareDistanceCalculator, settings, numberOfNeighboursConsidering, false);
            }
            if (ImGui::Selectable("Big MNIST"))
            {
                MNISTReader mnistReader;
                mnistReader.readData(tsne, squareDistanceCalculator, settings, numberOfNeighboursConsidering, true);
            }
            if (ImGui::Selectable("Myeloid8000"))
            {
                MyeloidReader myeloidReader;
                myeloidReader.readData(tsne, squareDistanceCalculator, settings, numberOfNeighboursConsidering);
            }
            if (ImGui::Selectable("Planaria"))
            {
                PlanariaReader planariaReader;
                planariaReader.readData(tsne, squareDistanceCalculator, settings, numberOfNeighboursConsidering);
            }

            ImGui::EndCombo();
        }

		static float accuracy = -1.0f;


        if (ImGui::Button("Compute accuracy") || glfwGetMouseButton(WindowHandler::getWindow(), GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
        {
            debugRenderData.clear();
			accuracy = tsne.getAccuracy();
        }

        if (accuracy != -1.0f)
        {
            ImGui::Text("Accuracy: %.2f", accuracy);
        }

		if (ImGui::Button("Compare exact and quadtree gradient computers"))
		{
			tsne.compareGradientComputers(quadtreeGradientComputer, exactGradientComputer, debugRenderData);
		}

        if (ImGui::Button("Compare exact and fitsne gradient computers"))
        {
			tsne.compareGradientComputers(exactGradientComputer, fItSNEGradientComputer, debugRenderData);
        }

        if (ImGui::Button("Compare quadtree and fitsne gradient computers"))
        {
			tsne.compareGradientComputers(quadtreeGradientComputer, fItSNEGradientComputer, debugRenderData);
        }

		ImGui::Checkbox("Run t-SNE", &runningTSNE);
        tooltip("Automatically keep running the t-SNE algorithm until this checkbox is unchecked");

		ImGui::Checkbox("Use label colors", &settings.useLabelColors);
		tooltip("Use the labels to color the points. If this is not checked, each point will simply have the same colour");

		ImGui::Checkbox("Enable debug", &debugRenderData.enabled);
		tooltip("Enable visual debugging information. Debug information can only be made in debugRenderData if this is turned on");

        ImGui::Checkbox("Show debug", &settings.showDebug);
		tooltip("Show any drawn debug information");

        ImGui::Checkbox("Follow data", &settings.followData);
		tooltip("Automatically follow the data when it moves");

		ImGui::SliderFloat("Theta", &quadtreeGradientComputer.theta, 0.1f, 3.0f);
		ImGui::DragInt("Bucket size", &quadtreeGradientComputer.bucketSize, 1.0f, 1, 1000);


		ImGui::Checkbox("Use PCA", &settings.usePCA);
		tooltip("Use PCA to set a more reasonable initial embedding of the high-dimensional data into 2D");


        if (ImGui::Button("Step t-SNE"))
        {
			TSNEGradientCompute& activeGradientComputer = getActiveGradientComputer(
                settings.activeGradientComputerIndex,
				exactGradientComputer,
				quadtreeGradientComputer,
				fItSNEGradientComputer,
				wrappedFItSNEGradientComputer);

            debugRenderData.clear();
            float tsneStepStartTime = static_cast<float>(glfwGetTime());
            tsne.step(activeGradientComputer, debugRenderData);
            float tsneStepEndTime = static_cast<float>(glfwGetTime());
            std::cout << "t-SNE step took " << tsneStepEndTime - tsneStepStartTime << " seconds" << std::endl;
            std::cout << "Running t-SNE at " << 1.0f / (tsneStepEndTime - tsneStepStartTime) << " steps / second" << std::endl;
        }
    	ImGui::Text("t-SNE step %d", tsne.getStepIndex());

		if (!AUTO_SAVE_SETTINGS && ImGui::Button("Save settings"))
		{
			writeSettingsToFile(settings);
		}

		if (AUTO_SAVE_SETTINGS && !compareSettings(savedSettings, settings))
		{
			std::cout << "Settings changed, writing to file..." << std::endl;
			writeSettingsToFile(settings);
		}

        // Write your user interface code here


        userInterface.end();


		// Output the current framebuffer to the window
        glfwSwapBuffers(WindowHandler::getWindow());
        glfwPollEvents();
    }

    Framework::terminate();

    glfwTerminate();

    return 0;
}
