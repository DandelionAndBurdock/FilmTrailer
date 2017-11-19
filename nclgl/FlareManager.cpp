#include "FlareManager.h"

#include "OpenGLUtility.h"

#include "../glm/gtx/matrix_transform_2d.hpp"

FlareManager::FlareManager()
{
}


FlareManager::~FlareManager()
{
}

void FlareManager::Render(const glm::vec3& cameraPos, const glm::mat4& projViewMat, const glm::vec3& lightPos) {
	// Convert sun/light position to screen coordinates
	glm::vec2 sunPos = WorldSpaceToScreenSpace(projViewMat, lightPos);
	if (sunPos.x < 0) { // Sun is not on the screen
		return;
	}
	// Vector from the sun to the centre of the screen in screen space
	glm::vec2 sunToCentre = SCREEN_CENTRE - sunPos;

	// Brightness decreases linearly with distance from sun up to the cut-off MIN_LIGHT_DISTANCE
	GLfloat brightness = 1.0 - glm::length(sunToCentre) / MIN_LIGHT_DISTANCE;

	// Calculate the flare positions
	if (brightness > 0) {
		SetFlarePositions(sunPos, sunToCentre);
	}

	// Render flares to screen

}


void FlareManager::SetFlarePositions(const glm::vec2& lightPos, const glm::vec2& lightToCentre) {
	for (int i = 0; i < flares.size(); ++i) {
		glm::vec2 sunToFlare = glm::scale(glm::mat3(), glm::vec2(GLfloat(i) * FLARE_SPACING)) * glm::vec3(lightToCentre, 1.0);
		flares[i].screenPos = lightPos + sunToFlare;
	}
}