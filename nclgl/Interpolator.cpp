#include "Interpolator.h"



Interpolator::Interpolator(const std::vector<glm::vec3>& wps) :
	waypoints(wps)
{
	numNodes = waypoints.size();
	time_between_nodes = total_circuit_time / numNodes;


}


Interpolator::~Interpolator()
{
}

void Interpolator::Update(float deltaSec) {
	elapsed += deltaSec;
	elapsed = fmod(elapsed, total_circuit_time);
	SetNodeIdxAndFactor();
}

void Interpolator::SetNodeIdxAndFactor()
{

	elapsed = fmod(elapsed, total_circuit_time);

	//Get the current node in elapsed time 
	float node_time = elapsed / time_between_nodes;

	//Get the current node index - floor(node_time)
	// and the factor - decimal part
	float node_time_floored;

	currentFactor = modf(node_time, &node_time_floored);
	currentIndex = (int)node_time_floored;
}

//Cubicly interpolates between b-c (with a and d being the nodes after/before the current segment)
glm::vec3 Interpolator::GetPos()
{
	//With factor between 0-1, this is defined as:
	// - See wiki Hermite Spline
	glm::vec3 posA = waypoints[(currentIndex + numNodes - 1) % numNodes];
	glm::vec3 posB = waypoints[currentIndex];
	glm::vec3 posC = waypoints[(currentIndex + 1) % numNodes];
	glm::vec3 posD = waypoints[(currentIndex + 1) % numNodes];

	//Get the tangents at points B and C
	// - These are the exit velocities of the two points, so if you change the 0.5f factor
	//   here you will either expand or shrink the amount of curvature. The half default
	//   means that the direction of the curve entering/leaving will be half way towards the
	//   the direction needed to hit the next point.
	glm::vec3 tanB = (posC - posA) * cubic_tangent_weighting;
	glm::vec3 tanC = (posD - posB) * cubic_tangent_weighting;

	float f2 = currentFactor * currentFactor;
	float f3 = f2 * currentFactor;

	return posB * (f3 * 2.0f - 3.0f * f2 + 1.0f)
		+ tanB * (f3 - 2.0f * f2 + currentFactor)
		+ posC * (-2.0f * f3 + 3.0f * f2)
		+ tanC * (f3 - f2);
}