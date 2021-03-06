#pragma once

#include "../../glm/vec3.hpp"

#include "Light.h"
class Spotlight : public Light
{
public:
	Spotlight(const glm::vec3& position, const glm::vec3& colour, const glm::vec3& direction);
	~Spotlight();


	glm::vec4 GetDirection() { return glm::vec4(direction, 1.0); }
	float GetInnerCutOff() { return innerCutoff; }
	float GetOuterCutOff() { return outerCutoff; }
	float GetRadius() { return radius; }
	void Randomise(float msec);
	void Rotate(const glm::mat4& rot);
protected:
	glm::vec3 direction;
	// Cosine of cutoff angle for spotlight cone in radians 
	// Angle between the spotlight direction and the outer cone
	float innerCutoffDegrees;
	float outerCutoffDegrees;
	float innerCutoff; 
	float outerCutoff;

	// Used for randomisation
	float lastChangedTime = 0.0f;						// Time since changed colour in milliseconds
	float colourChangeTime = 1000.0f;	// Time

	
};
