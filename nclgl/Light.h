#pragma once

#include "../glm/vec4.hpp"
#include "../glm/vec3.hpp"

#include "../GLEW/include/GL/glew.h"

// Maximum number of lights to be forward rendered at one time
#define MAX_LIGHTS  4

class Mesh;
#include "SceneNode.h"
#include "../glm/gtx/transform.hpp"	
class Light : public SceneNode
{
public:
	Light(const glm::vec3& position, const glm::vec3& colour, float radius) :
		SceneNode(cube, "LightShader")
	{
		SetModelScale(glm::vec3(7.5f));
		this->position = position;
		this->lightColour = colour;
		this->radius = radius;
		/*transform = glm::translate(position);
		SetModelScale(glm::vec3(10.0f));*/
	}

	~Light() {}

	bool operator<(const Light& rhs) const {
		return (this->distanceFromCamera > rhs.distanceFromCamera);
	}

	glm::vec4 GetPosition() const;
	void SetPosition(glm::vec3 val) {	position = val; }

	float GetRadius() const {	return radius;}
	void SetRadius(float val) {	radius = val; }

	glm::vec4 GetColour() const {	return glm::vec4(lightColour, 1.0); }
	void SetColour(glm::vec3 val) { lightColour = val; }

	static bool CreateLightMesh();


protected:
	static Mesh* cube;
	glm::vec3 position;
	glm::vec3 lightColour;
	GLfloat radius;
};
