#pragma once

#include "../../glm/vec3.hpp"
class Spotlight 
{
public:
	Spotlight(glm::vec3 position, glm::vec3 direction);
	~Spotlight();

	glm::vec3 GetPosition() { return position; }
	glm::vec3 GetDirection() { return direction; }
	glm::vec3 GetColour() { return colour; }
	float GetCutOff() { return cutoff; }
	float GetRadius() { return radius; }
protected:
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 colour;

	// Cutoff angle for spotlight cone in radians 
	// Angle between the spotlight direction and the outer cone
	float cutoff; 
	float radius;
	
};
