#pragma once

#include "../glm/glm.hpp"
// Convert world position to screen coordinates
glm::vec2 WorldSpaceToScreenSpace(const glm::mat4& projview, const glm::vec3& worldPos);
glm::vec2 WorldSpaceToScreenSpace(const glm::mat4& proj, const glm::mat4& view, const glm::vec3& worldPos);

