#pragma once

#include "../glm/vec4.hpp"
#include "../glm/vec3.hpp"

class Light
{
public:
	Light(glm::vec3 position, glm::vec4 colour, float radius) {
		this->position = position;
		this->colour = colour;
		this->radius = radius;
	}

	~Light() {}

	glm::vec3 GetPosition() const {	return position; }
	void SetPosition(glm::vec3 val) {	position = val; }

	float GetRadius() const {	return radius;}
	void SetRadius(float val) {	radius = val; }

	glm::vec4 GetColour() const {	return colour; }
	void SetColour(glm::vec4 val) { colour = val; }

protected:
	glm::vec3 position;
	glm::vec4 colour;
	float radius;
};
