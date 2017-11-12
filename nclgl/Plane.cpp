#include "Plane.h"

#include "../../glm/glm.hpp"

Plane::Plane(const glm::vec3& normal, float distance, bool normalise)
{
	if (normalise) {
		float length = normal.length();
		this->normal = normal / length;
		this->distance = distance / length;
	}
	else {
		this->normal = normal;
		this->distance = distance;
	}
}


Plane::~Plane()
{
}


bool Plane::SphereInPlane(const glm::vec3& position, float radius) const {
	float x = glm::dot(position, normal);
	if (glm::dot(position, normal) + distance <= -radius) {
		return false;
	}
	return true;
}