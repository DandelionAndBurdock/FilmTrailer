#pragma once

#include <vector>

#include "..\GLEW\include\GL\glew.h"

class Mesh;

class PostProcessor
{
public:
	PostProcessor(GLuint screenWidth, GLuint screenHeight);
	~PostProcessor();

	void BindSceneFBO();
	void ProcessScene();
	void BindProcessedTexture();

protected:
	void GaussianBlur(GLuint startTexture);
	void Contrast(GLuint startTexture);
	void Bloom(GLuint startTexture);

	void Initialise();
	void InitialiseSceneFBO();
	void InitialiseProcessFBO();

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
};

