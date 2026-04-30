#pragma once

// Matrix math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "WindowHandler.h"

class Camera
{
public:
	Camera();
	Camera(glm::vec3 pos);
	Camera(glm::vec3 pos, float yaw, float pitch);
	Camera(glm::vec3 pos, float yaw, float pitch, float sensitivity, float fov, float cameraSpeed);
	~Camera();

	void updateVectors();

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;

	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	float getPitch() const;
	float getYaw() const;
	glm::vec3 getForward() const;
	float getFov() const;

	void setPosition(glm::vec3 position);
	void setPitch(float newPitch);
	void setYaw(float newYaw);
	void setFov(float newFov);

	// Don't use the current mouse offset, but start from where the cursor is
	void resetMouseOffset();

	glm::vec3 getRayDirection(glm::vec2 coordinates);

	// Process the input to the camera.
	// Returns whether the camera moved.
	bool processInput(float deltaTime);

	// Get the ratio width/height
	float getAspectRatio();

	// Get important information of this camera (position, rotation)
	std::string getInformation();

	void onScroll(float delta);

	float getNearPlane();
	float getFarPlane();

	void setGlobalRotationPoint(glm::vec3 rotationPoint);

	glm::vec2 getLastMove();

	/* Private members */
private:

	// Different movement modes
	bool processInputGlobal(float xoffset, float yoffset, float scroll, float deltaTime);

	glm::vec3 calculateDirectionVector(float yaw, float pitch);

	// Camera postion data
	glm::vec3 position = glm::vec3(-4.0f, 0.5f, 0.0f);
	glm::vec3 up;
	glm::vec3 forward;
	float lastx = 400, lasty = 300;
	float yaw{ 0.0f }, pitch{ 0.0f };
	bool firstMouse = true;

	float nearPlane{ 0.1f };
	float farPlane{ 1000.0f };

	float sensitivity = 1.0f;
	float fov = 70.0f;
	float cameraSpeed = 1.0f;

	float scroll{ 0.0f };

	// Saved mouse offsets
	glm::vec2 lastMove{ 0.0f };

	glm::vec3 globalRotationPoint{ glm::vec3(0.0f) };
	float distanceToRotationPoint{ 3.0f };
};