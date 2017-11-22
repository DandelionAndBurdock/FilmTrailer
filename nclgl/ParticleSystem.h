//http://ogldev.atspace.co.uk/www/tutorial28/tutorial28.html
#pragma once

#include "../../glm/vec3.hpp"
#include "../../glm/mat4x4.hpp"

#include "../../GLEW/include/GL/glew.h"


#include <string>

//TODO: Make this a derived class
class FireworkSystem
{
public:
	FireworkSystem(const glm::vec3& pos);
	~FireworkSystem();

	void UpdateParticles(GLfloat msec);
	void Render(const glm::mat4& viewProj, const glm::vec3 cameraPos);
protected:
	void InitFireworks(const glm::vec3& pos);
	void LoadShaders();
	
	void RenderParticles(const glm::mat4& viewProj, const glm::vec3 cameraPos);

	GLboolean isFirst;
	GLuint currentVBO;					// Current vertex buffer index
	GLuint currentFBO;					// Current feedback buffer index

	GLuint transfromFeedBackBuffer[2];  // Will ping-pong between two TF buffers for drawing and reading
	GLuint particleBuffer[2];			// One particle buffer for each TF buffer

	// Time in milliseconds
	GLfloat time;

	GLuint updateVAO;
	GLuint billboardVAO;

	std::string updateShader;
	std::string billboardShader;

	std::string texture;
};