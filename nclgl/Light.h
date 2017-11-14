#pragma once

#include "../glm/vec4.hpp"
#include "../glm/vec3.hpp"

#include "../GLEW/include/GL/glew.h"

// Maximum number of lights to be forward rendered at one time
#define MAX_LIGHTS  4

class Light
{
public:
	Light(glm::vec3 position, glm::vec4 colour, float radius) {
		this->position = position;
		this->colour = colour;
		this->radius = radius;
	}

	~Light() {}

	bool operator<(const Light& rhs) const {
		return (this->cameraDistance > rhs.cameraDistance);
	}

	glm::vec3 GetPosition() const {	return position; }
	void SetPosition(glm::vec3 val) {	position = val; }

	float GetRadius() const {	return radius;}
	void SetRadius(float val) {	radius = val; }

	glm::vec3 GetColour() const {	return colour; }
	void SetColour(glm::vec3 val) { colour = val; }

protected:
	glm::vec3 position;
	glm::vec3 colour;
	GLfloat radius;

	GLfloat cameraDistance;
};
