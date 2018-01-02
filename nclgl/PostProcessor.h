#pragma once

#include <vector>

#include "..\GLEW\include\GL\glew.h"
#include "../glm/vec3.hpp"
class Mesh;

class PostProcessor
{
public:
	PostProcessor(GLuint screenWidth, GLuint screenHeight);
	~PostProcessor();

	void BindSceneFBO();
	void ProcessScene();
	void BindProcessedTexture();

	void BindSpecialFBO();

	void BlurOn() { blurOn = true; }
	void BlurOff() { blurOn = false; }
	void ShatterOn() { shatterOn = true; }
	void ShatterOff() { shatterOn = false; }
	void ShatterMoveOn() { shatterMove = true; }
	void ShatterMoveOff() { shatterMove = false; }
	void SetBlurPasses(GLint n) { blurPasses = n; }
	void Update(GLfloat deltaSec);


protected:
	void GaussianBlur(GLuint startTexture);
	void Contrast(GLuint startTexture);
	void Bloom(GLuint startTexture);
	void Combine(GLuint sceneTexture, GLuint highlightTexture);
	void Shatter();

	void Initialise();
	void InitialiseSceneFBO();
	void InitialiseProcessFBO();
	void InitaliseSpecialFBO();

	GLuint sceneFBO;
	GLuint sceneColourTex;
	GLuint sceneDepthTex;

	GLuint processFBO;
	GLuint processColourTex[2];

	GLuint texWidth;
	GLuint texHeight;

	GLuint finalProcessTex;

	GLuint blurPasses;

	GLfloat contrastLevel;

	Mesh* sceneQuad;

	GLboolean blurOn = false;

	// Shatter variables
	GLboolean shatterOn = false;
	GLboolean shatterMove = false;
	glm::vec3 velocityO = glm::vec3(-0.1f, -0.45f, 0.0f);
	glm::vec3 velocityA = glm::vec3(-0.1f, -0.2f, 0.0f);
	glm::vec3 velocityB = glm::vec3(0.05f, -0.2f, 0.0f);
	glm::vec3 velocityC = glm::vec3(0.1f, -0.4f, 0.0f);
	glm::vec3 velocityD = glm::vec3(0.2f, -0.2f, 0.0f);
	glm::vec3 velocityE = glm::vec3(-0.1f, -0.02f, 0.0f);
	glm::vec3 velocityF = glm::vec3(0.15f, -0.3f, 0.0f);
	glm::vec3 velocityG = glm::vec3(-0.1f, -0.2f, 0.0f);
	GLfloat time = 0.0f;
	glm::vec3 gravity = glm::vec3(0.0f, 0.1f, 0.0f);

	GLfloat angle = 0.0f;
	GLfloat angleA = 0.0f;
	GLfloat angleB= 0.0f;
	GLfloat angleC = 0.0f;
	GLfloat angleD = 0.0f;
	GLfloat angleE = 0.0f;
	GLfloat angleF = 0.0f;
	GLfloat angleG = 0.0f;

	GLuint specialFBO;
	GLuint specialColourTex;
	GLuint specialDepthTex;
};

