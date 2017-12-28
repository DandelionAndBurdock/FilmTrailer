#pragma once

#include <vector>
#include "../glm/vec3.hpp"

class Interpolator
{
public:
	Interpolator(const std::vector<glm::vec3>& waypoints);
	~Interpolator();

	void Update(float deltaSec);
	glm::vec3 GetPos();

	void SetWaypoints(const std::vector<glm::vec3>& wps, float circuitTime) 
	{ 
		waypoints = wps; 
		numNodes = wps.size(); 
		total_circuit_time = circuitTime;
		time_between_nodes = total_circuit_time / numNodes; 
		elapsed = 0.0f; 
	}
	float GetCircuitTime() { return total_circuit_time - time_between_nodes; }
protected:

	void SetNodeIdxAndFactor();

	float total_circuit_time = 7.5f;
	float time_between_nodes;
	// Elapsed time in seconds
	float elapsed = 0.0f;
	int currentIndex;
	float currentFactor;
	int numNodes;
	std::vector<glm::vec3> waypoints;

	float cubic_tangent_weighting = 0.5f;
};


