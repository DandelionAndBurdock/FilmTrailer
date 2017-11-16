#pragma once

#include "../glm/glm.hpp"
#include "../../GLEW/include/GL/glew.h"


class DirectionalLight
{
public:
	DirectionalLight(glm::vec3 dir, glm::vec3 col) :
		direction(dir), colour(col) {}
	~DirectionalLight() {}

	// Rotates the direction theta degrees about axis
	void Rotate(GLfloat theta, glm::vec3 axis);

	glm::vec4 GetDirection() { return glm::vec4(direction, 1.0f); }
	glm::vec4 GetColour() { return glm::vec4(colour, 1.0f); }

protected:
	glm::vec3 direction;	// Direction of parallel light rays in world space
	glm::vec3 colour;
};

