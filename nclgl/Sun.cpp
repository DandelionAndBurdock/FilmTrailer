#include "Sun.h"

#include "DirectionalLight.h"

Sun::Sun(glm::vec3 direction, glm::vec3 colour, GLfloat distance) :
	SceneNode(Mesh::GenerateSimpleQuad(), "SunShader"),
	light(new DirectionalLight(direction, colour))
{
	position = -direction * 1000.0f; //TODO: Sun distance
}


Sun::~Sun()
{
	delete light;
	light = nullptr;
}
