#include "CameraController.h"

#include "Camera.h"

#include <iostream>

CameraController::CameraController(Camera* camera, std::vector<glm::vec3>& wayPoints, std::vector<glm::vec3>& lookPoints, std::vector<float>& tps) :
	camera(camera), wayPoints(wayPoints), viewPoints(lookPoints), timePoints(tps)
{
	if (wayPoints.size() != lookPoints.size()) {
		std::cout << "Error initalising camera controller" << std::endl;
		camera = nullptr;
		return;
	}
	time = 0.0f;
	currentWaypoint = 0;
	nextWaypoint = (currentWaypoint + 1) % wayPoints.size();
	timePerWayPoint = tps[currentWaypoint];
	automatic = true;
}

CameraController::~CameraController()
{
}


void CameraController::Update(float msec) {
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_Q)) {
		ToggleAutoMovement();
	}
	if (automatic) {
		UpdateTime(msec);
		UpdatePosition();
		UpdateViewDirection();
	}
	else {
		camera->UpdateCamera(msec);
	}
}

void CameraController::UpdateTime(float msec) {
	time += msec;

	if (time > timePerWayPoint) { //TODO: Would have problems with very small timePerWayPoint < frameRate
		time = time - timePerWayPoint;
		currentWaypoint = (currentWaypoint + 1) % wayPoints.size();
		nextWaypoint = (currentWaypoint + 1) % wayPoints.size();
		timePerWayPoint = timePoints[currentWaypoint % timePoints.size()];
	}
}

void CameraController::ToggleAutoMovement() {
	automatic = !automatic;

	if (!automatic) {
		time = 0.0f;
		currentWaypoint = 0;
	}

}

template <typename T>
T lerp(T a, T b, float mix) {
	return (1 - mix) * a + mix * b;
}


#include <iostream>
void CameraController::UpdatePosition() {
	glm::vec3 targetPosition(lerp<glm::vec3>(wayPoints[currentWaypoint], wayPoints[nextWaypoint], time / timePerWayPoint));
	glm::vec3 currentPosition = camera->GetPosition();
	camera->SetPosition(lerp<glm::vec3>(currentPosition, targetPosition, time / timePerWayPoint));
}

void CameraController::UpdateViewDirection() {
	glm::vec3 targetDirection = lerp<glm::vec3>(viewPoints[currentWaypoint], viewPoints[nextWaypoint], time / timePerWayPoint);
	glm::vec3 currentDirection = camera->GetViewDirection();
	camera->SetViewDirection(glm::normalize(lerp<glm::vec3>(currentDirection, targetDirection, time / timePerWayPoint)));
}