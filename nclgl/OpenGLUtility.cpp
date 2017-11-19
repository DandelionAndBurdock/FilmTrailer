#include "OpenGLUtility.h"

glm::vec2 WorldSpaceToScreenSpace(const glm::mat4& projview, const glm::vec3& worldPos) {
	glm::vec4 clipSpace = projview * glm::vec4(worldPos, 1.0);
	// Persective division to NDC
	if (clipSpace.w <= 0.0) { // Not on screen 
		return glm::vec2(-1.0f);
	}
	else {
		glm::vec2 NDC = glm::vec2(clipSpace.x, clipSpace.y) / clipSpace.w;
		// Move from NDC to uv coords
		glm::vec2 uv = (NDC + glm::vec2(1.0f)) / 2.0f;
		// Flip y-axis so that (0, 0) is at the top
		return glm::vec2(uv.x, 1 - uv.y);
	}
}
glm::vec2 WorldSpaceToScreenSpace(const glm::mat4& proj, const glm::mat4& view, const glm::vec3& worldPos) {
	glm::mat4 projView = proj * view;
	return WorldSpaceToScreenSpace(projView, worldPos);
}