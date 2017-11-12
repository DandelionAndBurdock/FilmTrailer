#include "Camera.h"

#include "../glm/gtx/transform.hpp" // For rotate() and translate()
#include "../glm/glm.hpp"

Camera::Camera(glm::vec3 pos, glm::vec3 viewDir, glm::vec3 upDir) {
	position = pos;
	up = upDir;
	viewDirection = viewDir;
	right = glm::normalize(glm::cross(viewDirection, up));
	cameraMoveSpeed = 0.5f;
	cameraRotateSpeed = 1.0f;
}
/*
Polls the camera for keyboard / mouse movement.
Should be done once per frame! Pass it the msec since
last frame (default value is for simplicities sake...)
*/
void Camera::UpdateCamera(float msec)	{
	HandleMouseUpdates();
	HandleKeyboardUpdates(msec);
}


void Camera::HandleMouseUpdates() {
	//Update the mouse by how much
	float deltaPitch = Window::GetMouse()->GetRelativePosition().y;
	float deltaYaw = Window::GetMouse()->GetRelativePosition().x;

	right = glm::normalize(glm::cross(viewDirection, up));
	viewDirection = glm::mat3(glm::rotate(glm::radians(-deltaYaw * cameraRotateSpeed), 
					up)) * viewDirection;
	viewDirection = glm::mat3(glm::rotate(glm::radians(-deltaPitch* cameraRotateSpeed), 
						right)) * viewDirection;
}

void Camera::HandleKeyboardUpdates(float msec) {

	float moveDistance = msec * cameraMoveSpeed;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += viewDirection * moveDistance;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= viewDirection * moveDistance;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position += right * moveDistance;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position -= right * moveDistance;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		position += up * moveDistance;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		position -= up * moveDistance;
	}
}


/*
Generates a view matrix for the camera's viewpoint. This matrix can be sent
straight to the shader...it's already an 'inverse camera' matrix.
*/
glm::mat4 Camera::BuildViewMatrix()	{
	return glm::lookAt(position, position + viewDirection, up);
};
