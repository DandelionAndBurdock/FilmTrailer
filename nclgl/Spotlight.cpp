#include "Spotlight.h"

#include "../glm/glm.hpp"

Spotlight::Spotlight(glm::vec3 position, glm::vec3 direction) :
	position(position), direction(direction)
{
	cutoff = glm::cos(glm::radians(12.5f)); //TODO: Remove magic number
	colour = glm::vec3(1.0f);
	radius = 10000.0f;
}


Spotlight::~Spotlight()
{
}
