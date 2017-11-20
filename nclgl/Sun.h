#pragma once


#include "SceneNode.h"
#include "../../glm/vec3.hpp"

class DirectionalLight;
class Sun : public SceneNode
{
public:
	Sun(glm::vec3 direction, glm::vec3 colour, GLfloat distance);
	~Sun();

	glm::vec3 GetPosition() { return position; }
	DirectionalLight* GetLight() { return light; }
protected:
	glm::vec3 position;
	DirectionalLight* light;
};

