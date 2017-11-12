#pragma once

#include "../../glm/vec3.hpp"

class Plane
{
public:
	Plane() {};
	Plane(const glm::vec3& normal, float distance, bool normalise = false);
	~Plane();

	void SetNormal(const glm::vec3& normal) { this->normal = normal; }
	glm::vec3 GetNormal() const { return normal; }

	void SetDistance(float dist) { distance = dist; }
	float GetDistance() const { return distance; }

	bool SphereInPlane(const glm::vec3& position, float radius) const;
protected:
	glm::vec3 normal;
	float distance;
};