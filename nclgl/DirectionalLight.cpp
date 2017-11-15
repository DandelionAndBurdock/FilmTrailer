#include "DirectionalLight.h"

#include "../../glm/gtx/transform.hpp"
#include <iostream>
void DirectionalLight::Rotate(GLfloat theta, glm::vec3 axis) {
	glm::mat3 rotate = glm::rotate(glm::radians(theta), axis);
	direction = glm::normalize(rotate * direction);
}