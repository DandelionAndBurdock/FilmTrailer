#pragma once
#include "../GLEW/include/GL/glew.h"

#include "../glm/mat4x4.hpp"

class SimpleShadow
{
	static const int NUM_MAT_DIMENSIONS = 4;
	static const float biasValues[NUM_MAT_DIMENSIONS * NUM_MAT_DIMENSIONS];
	const int SHADOW_SIZE = 2048;
public:
	SimpleShadow(GLuint screenWidth, GLuint screenHeight);
	~SimpleShadow();

	void BindShadow();
	void UnbindShadow();
	void BindForReading();

	glm::mat4 GetBiasMatrix() { return biasMatrix; }

protected:
	GLuint shadowFBO;
	GLuint shadowDepthTex;

protected:
	GLuint screenWidth;
	GLuint screenHeight;

	glm::mat4 biasMatrix;
};

