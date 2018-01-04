// Controls automatic movement of camera by
// sending camera between waypoints

#pragma once

#include <vector>
#include "../glm/vec3.hpp"

class Camera;

class CameraController
{
public:
	CameraController(Camera* camera, std::vector<glm::vec3>& wayPoints, std::vector<glm::vec3>& lookAt, std::vector<float>& tps);
	~CameraController();

	void Update(float msec);
	void ToggleAutoMovement();

protected:
	void UpdateTime(float msec);
	void UpdatePosition();
	void UpdateViewDirection();

	std::vector<glm::vec3> wayPoints;
	std::vector<glm::vec3> viewPoints; //TODO: Change to look at points
	std::vector<float> timePoints;
	Camera* camera;

	bool automatic;

	std::vector<glm::vec3>::size_type currentWaypoint;
	std::vector<glm::vec3>::size_type nextWaypoint;
	
	float timePerWayPoint = 10000.0f; // Time in milliseconds
	float time; //Total time for current waypoint transition
};

