#include "Camera.h"

Camera::Camera()
{
	updateVectors();
	position = globalRotationPoint - distanceToRotationPoint * getForward();
}
Camera::Camera(glm::vec3 pos)
	: position(pos)
{
	updateVectors();
}
Camera::Camera(glm::vec3 pos, float yaw, float pitch)
	: position(pos)
	, yaw(yaw)
	, pitch(pitch)
{
	updateVectors();
}
Camera::Camera(glm::vec3 pos, float yaw, float pitch, float sensitivity, float fov, float cameraSpeed)
	: position(pos)
	, yaw(yaw)
	, pitch(pitch)
	, sensitivity(sensitivity)
	, fov(fov)
	, cameraSpeed(cameraSpeed)
{
	updateVectors();
}
Camera::~Camera()
{
}

void Camera::updateVectors()
{
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward = glm::normalize(direction);
}

glm::mat4 Camera::getViewMatrix() const
{
	glm::mat4 view = glm::lookAt(position, position + forward, up);
	return view;
}
glm::mat4 Camera::getProjectionMatrix() const
{
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WindowHandler::getWindowSize().x / (float)WindowHandler::getWindowSize().y, nearPlane, farPlane);
	return projection;
}

glm::vec3 Camera::getPosition() const
{
	return glm::vec3(position.x, position.y, position.z);
}
glm::vec3 Camera::getRotation() const
{
	return glm::vec3(0.0f, glm::radians(yaw), -glm::radians(pitch));
}

float Camera::getPitch() const
{
	return pitch;
}

float Camera::getYaw() const
{
	return yaw;
}

glm::vec3 Camera::getForward() const
{
	return glm::normalize(forward);
}

float Camera::getFov() const
{
	return fov;
}

void Camera::setPosition(glm::vec3 position)
{
	this->position = position;
	updateVectors();
}

void Camera::setPitch(float newPitch)
{
	this->pitch = newPitch;
	updateVectors();
}

void Camera::setYaw(float newYaw)
{
	this->yaw = newYaw;
	updateVectors();
}

void Camera::setFov(float newFov)
{
	this->fov = newFov;
	updateVectors();
}

float Camera::getAspectRatio()
{
	return (float)WindowHandler::getWindowSize().x / (float)WindowHandler::getWindowSize().y;
}

std::string Camera::getInformation()
{
	return "Camera position: ("
		+ std::to_string(getPosition().x) + ", "
		+ std::to_string(getPosition().y) + ", "
		+ std::to_string(getPosition().z) +
		"), pitch: " + std::to_string(pitch)
		+ ", yaw: " + std::to_string(yaw);
}

void Camera::onScroll(float delta)
{
	scroll += delta;
}

float Camera::getNearPlane()
{
	return nearPlane;
}

float Camera::getFarPlane()
{
	return farPlane;
}

void Camera::setGlobalRotationPoint(glm::vec3 rotationPoint)
{
	globalRotationPoint = rotationPoint;
}

glm::vec2 Camera::getLastMove()
{
	return lastMove;
}

bool Camera::processInputGlobal(float xoffset, float yoffset, float scroll, float deltaTime)
{
	// Saving the previous camera position to compare to the new to determine if the camera moved
	glm::vec3 prevPosition = position;

	bool shiftHeld{ glfwGetKey(WindowHandler::getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS };

	globalRotationPoint = position + distanceToRotationPoint * getForward();

	bool moveKeyHeld{ glfwGetMouseButton(WindowHandler::getWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS };
	glm::vec3 toRotationPoint{ globalRotationPoint - getPosition() };

	glm::vec3 right{ glm::cross(forward, up) };
	glm::vec3 localUp{ glm::cross(forward, right) };

	xoffset *= sensitivity * 0.1f;
	yoffset *= sensitivity * 0.1f;

	lastMove = glm::vec2(0.0f);
	if (moveKeyHeld)
	{
		lastMove = glm::vec2(xoffset, yoffset);

		// Movement
		if (shiftHeld)
		{
			position -= right * (float)xoffset * 0.02f * std::max(distanceToRotationPoint, 0.1f);
			position += localUp * (float)yoffset * 0.02f * std::max(distanceToRotationPoint, 0.1f);
		}
		// Rotate around object
		else
		{
			// Applying rotation
			float yawBefore{ yaw };
			float pitchBefore{ pitch };
			yaw += xoffset;
			pitch += yoffset;

			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			float deltaYaw{ yaw - yawBefore };
			float deltaPitch{ pitch - pitchBefore };

			// Recalculating the forward vector
			forward = calculateDirectionVector(yaw, pitch);

			// Rotate around with offset
			toRotationPoint = glm::rotate(toRotationPoint, glm::radians(deltaPitch), right);
			toRotationPoint = glm::rotate(toRotationPoint, -glm::radians(deltaYaw), glm::vec3(0.0f, 1.0f, 0.0f));

			position = globalRotationPoint - toRotationPoint;
		}
	}

	// Movement by scrolling
	if (shiftHeld)
	{
		fov -= scroll * sensitivity;

		fov = glm::clamp(fov, 10.0f, 120.0f);
	}
	else
	{
		distanceToRotationPoint += -scroll * sensitivity * 0.2f * std::max(distanceToRotationPoint, 0.1f);
		position = globalRotationPoint - forward * distanceToRotationPoint;
	}

	// Return that the camera moved if its position is not the same anymore
	return prevPosition != position;
}

void Camera::resetMouseOffset()
{
	firstMouse = true;
}

glm::vec3 Camera::getRayDirection(glm::vec2 coordinates)
{
	glm::vec3 dir;

	float d = 1 / tan(glm::radians(fov) / 2);
	float aspect_ratio = (float)WindowHandler::getWindowSize().x / (float)WindowHandler::getWindowSize().y;

	dir.x = aspect_ratio * (2 * coordinates.x / (float)WindowHandler::getWindowSize().x) - aspect_ratio;
	dir.y = -((2.0f * coordinates.y / (float)WindowHandler::getWindowSize().y) - 1.0f);
	dir.z = d;
	dir = normalize(dir);

	float a = getRotation().x;
	float b = getRotation().y;
	float c = getRotation().z;
	// Precalculating sin
	float sa = sin(a);
	float sb = sin(b);
	float sc = sin(c);
	// Precalculating cos
	float ca = cos(a);
	float cb = cos(b);
	float cc = cos(c);

	// Calculating ray direction
	float   x = dir.x * ca * cb;
	x += dir.y * (ca * sb * sc - sa * cc);
	x += dir.z * (ca * sb * cc + sa * sc);

	float   y = dir.x * sa * cb;
	y += dir.y * (sa * sb * sc + ca * cc);
	y += dir.z * (sa * sb * cc - ca * sc);

	float   z = dir.x * -sb;
	z += dir.y * cb * sc + dir.z * cb * cc;

	dir = glm::vec3(z, y, x);

	return glm::normalize(dir);
}

// Processes the input
bool Camera::processInput(float deltaTime)
{
	// Getting the mouse position
	double xpos, ypos;
	glfwGetCursorPos(WindowHandler::getWindow(), &xpos, &ypos);
	glm::vec2 mousePos = glm::vec2(xpos, ypos);

	// Sets the mouse offset by frame appropriately for the first frame
	if (firstMouse)
	{
		lastx = mousePos.x;
		lasty = mousePos.y;
		firstMouse = false;
	}

	float xoffset = mousePos.x - lastx;
	float yoffset = lasty - mousePos.y; // reversed since y-coordinates range from bottom to top
	lastx = mousePos.x;
	lasty = mousePos.y;

	// Capturing the scroll deltas
	float scrollDelta{ 0.0f };

	glm::vec3 positionBefore = position;
	float yawBefore = yaw;
	float pitchBefore = pitch;

	bool moved = false;

	moved = processInputGlobal(xoffset, yoffset, scrollDelta, deltaTime);

	glm::vec3 positionDelta = position - positionBefore;

	float deltaYaw = yaw - yawBefore;
	float deltaPitch = pitch - pitchBefore;

	return moved;
}

glm::vec3 Camera::calculateDirectionVector(float yaw, float pitch)
{
	glm::vec3 direction;

	// Calculate the direction vector using spherical coordinates
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	// Normalize the direction vector
	direction = glm::normalize(direction);

	return direction;
}