#include "Spotlight.h"

#include "../glm/glm.hpp"

#include "RandomNumberGenerator.h"

Spotlight::Spotlight(const glm::vec3& position, const glm::vec3& colour, const glm::vec3& direction) :
	Light(position, colour, 1000.0f), direction(direction)
{
	outerCutoffDegrees = 50.0f;
	innerCutoffDegrees = 0.4 * outerCutoffDegrees;
	innerCutoff = glm::cos(glm::radians(innerCutoffDegrees)); //TODO: Remove magic number
	outerCutoff = glm::cos(glm::radians(outerCutoffDegrees));
}


Spotlight::~Spotlight()
{
}
#include "../glm/gtx/transform.hpp"
#include <iostream>
#include "ShaderManager.h"
void Spotlight::Randomise(float msec) {
	lastChangedTime += msec;
	if (lastChangedTime > colourChangeTime) {
		lastChangedTime -= colourChangeTime;
		colourChangeTime = RNG->GetRandFloat(1500.0f, 3000.0f);
		lightColour = RNG->GetRandomColour();
		SHADER_MANAGER->SetUniform("ConeShader", "coneColour", lightColour);
	}
	//TODO: Remove magic number
	//TODO: Make function
	const float outerCutoffDegreesMax = 50.0f;
	const float outerCutoffDegreesMin = 10.0f;
	////////outerCutoffDegrees = RNG->GetRandFloat(10.0f, 40.0f);
	////////std::cout << outerCutoffDegrees << std::endl;
	////////innerCutoffDegrees = outerCutoffDegrees * 0.4;
	////////innerCutoff = glm::cos(glm::radians(3.0f));
	////////outerCutoff = glm::cos(glm::radians(7.0f));

	//direction.x += msec * RNG->GetRandFloat(-0.004, 0.004);
	//direction.z += msec * RNG->GetRandFloat(-0.004, 0.004);
	//direction = glm::rotate(0.1f, glm::vec3(0.0, 0.0, 1.0)) * glm::vec4(direction, 1.0);
	
}

void Spotlight::Rotate(const glm::mat4& rot) {
	direction = glm::vec3(rot * glm::vec4(0.0f, -1.0f, 0.0f, 1.0f));
	direction = glm::normalize(direction);
	//std::cout << "Direction: " << direction.x << "," << direction.y << "," << direction.z << std::endl;
}