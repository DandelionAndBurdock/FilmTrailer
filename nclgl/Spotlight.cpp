#include "Spotlight.h"

#include "../glm/glm.hpp"

#include "RandomNumberGenerator.h"

Spotlight::Spotlight(const glm::vec3& position, const glm::vec3& colour, const glm::vec3& direction) :
	Light(position, colour, 1000.0f), direction(direction)
{
	innerCutoffDegrees = 5.0f;
	outerCutoffDegrees = 10.0f;
	innerCutoff = glm::cos(glm::radians(innerCutoffDegrees)); //TODO: Remove magic number
	outerCutoff = glm::cos(glm::radians(outerCutoffDegrees));

}


Spotlight::~Spotlight()
{
}


void Spotlight::Randomise(float msec) {
	lastChangedTime += msec;
	if (lastChangedTime > colourChangeTime) {
		lastChangedTime -= colourChangeTime;
		lightColour = RNG->GetRandomColour();
		colourChangeTime = RNG->GetRandFloat(500.0f, 1500.0f);
	}
	//TODO: Remove magic number
	//std::cout << "Pre-" << outerCutoffDegrees << std::endl;
	//outerCutoffDegrees += msec * RNG->GetRandFloat(-2 / 1000.0f, 2 / 1000.0f);
	//std::cout << "Post" << outerCutoffDegrees << std::endl;
	//innerCutoffDegrees = outerCutoffDegrees * 0.4;
	//innerCutoff = glm::cos(glm::radians(3.0f));
	//outerCutoff = glm::cos(glm::radians(7.0f));

	//direction.x += msec * RNG->GetRandFloat(0.0, 0.001);
	//direction.z += msec * RNG->GetRandFloat(0.0, 0.001);
	//glm::normalize(direction);
}