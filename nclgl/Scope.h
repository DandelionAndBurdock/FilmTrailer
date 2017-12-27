#pragma once

#include <GL\glew.h>
#include "../glm/vec2.hpp"
#include "../glm/mat4x4.hpp"

class Shader;
class Mesh;
class Texture;

class Scope
{
	// Scope will appear after SWITCH_ON_TIME seconds
	const float SWITCH_ON_TIME = 0.0f;

	// Number of points on the circle circumference
	const int NUM_POINTS = 40;

	//const float 
public:
	Scope(GLfloat screenWidth, GLfloat screenHeight);
	~Scope();

	void DrawCircle();
	void UpdateCircle(GLfloat timeSec);
	void DrawCrossHair();
protected:
	// Centre of the circle in screen space
	glm::vec2 centre = glm::vec2(0.0f);

	// Contains points of the circle
	GLfloat* circleVertices;

	// Indices to draw the circle
	GLuint* circleIndices;

	// Rebuffer vertex data to openGL
	void RebufferVertices();
	
	// Vertex Array Object for circle
	GLuint circleVAO;

	// Vertex Buffer Object for vertices
	GLuint vertexBufferID;

	// Time in seconds
	GLfloat time = 0.0f;

	// Shader to draw circle
	Shader* shader;

	// Shader to draw cross-hair
	Shader* crossShader;

	// Projection Matrix
	glm::mat4 projMatrix;

	// num Indices
	GLint numIndices = 0;

	// Radius of the circle in screen space
	const float radius = 0.4f;

	// Cross hair quad
	Mesh* quad;

	// Cross hair texture
	Texture* crossTexture;
};


