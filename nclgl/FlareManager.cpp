#include "FlareManager.h"

#include "OpenGLUtility.h"
#include "Mesh.h"

#include "../glm/gtx/matrix_transform_2d.hpp"

#include "FlareManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"

FlareManager::FlareManager()
{
	shader = "FlareShader";
	LoadFlares();
}


FlareManager::~FlareManager()
{
}

void FlareManager::PrepareToRender(const glm::vec3& cameraPos, const glm::mat4& projViewMat, const glm::vec3& lightPos) {
	// Convert sun/light position to screen coordinates
	glm::vec2 sunPos = WorldSpaceToScreenSpace(projViewMat, lightPos);
	if (sunPos.x < 0) { // Sun is not on the screen
		return;
	}
	// Vector from the sun to the centre of the screen in screen space
	glm::vec2 sunToCentre = SCREEN_CENTRE - sunPos;

	// Brightness decreases linearly with distance from sun up to the cut-off MIN_LIGHT_DISTANCE
	brightness = 1.0 - glm::length(sunToCentre) / MIN_LIGHT_DISTANCE;

	// Calculate the flare positions
	if (brightness > 0) {
		SetFlarePositions(sunPos, sunToCentre);
		SHADER_MANAGER->SetUniform(shader, "brightness", brightness);
	}
}

void FlareManager::Render() {
	if (brightness < 0) {
		return;
	}
	for (auto& flare : flares) {
		flare.Render();
	}
}

void FlareManager::LoadFlares() {
	const int NUM_FLARES = 11;
	GLfloat scales[NUM_FLARES] = { 0.1f, 0.15f, 0.05f, 0.01f, 0.01f, 0.015f, 0.05f, 0.015f, 0.1f, 0.3f, 0.15f };
	GLint textures[NUM_FLARES] = { 5, 3, 1, 6, 2, 4, 6, 2, 4, 3, 7 };
	for (int i = 0; i < NUM_FLARES; ++i) {
		flares.push_back(Flare("Flare" + std::to_string(textures[i]), scales[i]));
	}
	
}

float FLARE_SPACINGS[11] = { 0.0, 3.0, 0.3, 0.8, 1.2, 1.6, 2.2, 2.4, 2.9, 3.1, 0.1 };
void FlareManager::SetFlarePositions(const glm::vec2& lightPos, const glm::vec2& lightToCentre) {
	for (int i = 0; i < flares.size(); ++i) {
		//glm::vec2 sunToFlare = glm::scale(glm::mat3(), glm::vec2(GLfloat(i) * FLARE_SPACING)) * glm::vec3(lightToCentre, 1.0);
		glm::vec2 sunToFlare = glm::scale(glm::mat3(), glm::vec2(FLARE_SPACINGS[i])) * glm::vec3(lightToCentre, 1.0);

		flares[i].screenPos = lightPos + sunToFlare;
	}
}


struct Flare {
	std::string texture;
	glm::vec2 screenPos;
	glm::vec2 scale;
	
};


FlareManager::Flare::Flare(const std::string& tex, GLfloat size)
	: texture(tex),screenPos(glm::vec2(0.0f)), scale(glm::vec2(size)) {
	shader = "FlareShader";
	quad = Mesh::GenerateSimpleQuad();
}


void FlareManager::Flare::Render() {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	SHADER_MANAGER->SetUniform(shader, "diffuseTex", 0);
	glActiveTexture(GL_TEXTURE0);
	TEXTURE_MANAGER->BindTexture(texture);
	glm::vec4 transform = glm::vec4(screenPos.x, screenPos.y, scale.x, scale.y);
	SHADER_MANAGER->SetUniform(shader, "transform", transform);
	quad->Draw();
	glEnable(GL_DEPTH_TEST);
}