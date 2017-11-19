// Thin Matrix https://www.dropbox.com/sh/vo7dwmaccrtc2ng/AAAr48L7pGsSXs0EEQtkWKspa?dl=0
#pragma once
#include "../glm/vec2.hpp"
#include "../../GLEW/include/GL/glew.h"
#include "../glm/mat4x4.hpp"
#include <string>
#include <vector>
class FlareManager
{
	// Centre of the screen in display coordinates (0, 0) top left (1,1) bottom right
	static constexpr  glm::vec2 SCREEN_CENTRE = glm::vec2(0.5f);
	// How far apart different lens flares can be rendered in units of distance from centre of screen to sun (or light source)
	static constexpr  GLfloat FLARE_SPACING = 0.2f;
	// How close to the light source in screen space to produce lens flare
	static constexpr  GLfloat MIN_LIGHT_DISTANCE = 0.6f;

	struct Flare {
		std::string texture;
		glm::vec2 screenPos;
		GLfloat scale;
		Flare(const std::string& str, )
	};
public:
	FlareManager();
	~FlareManager();

	void Render(const glm::vec3& cameraPos, const glm::mat4& projViewMat, const glm::vec3& lightPos);

protected:
	std::vector<Flare> flares;

	void SetFlarePositions(const glm::vec2& lightPos, const glm::vec2& lightToCentre);
};

