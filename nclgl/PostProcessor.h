#pragma once

#include <vector>

#include "..\GLEW\include\GL\glew.h"
class PostProcessor
{
public:
	PostProcessor(GLuint screenWidth, GLuint screenHeight);
	~PostProcessor();

	void BindSceneFBO();
	void ProcessScene();
	void BindProcessedTexture();

protected:
	void Initialise();
	void InitialiseSceneFBO();
	GLuint sceneFBO;
	GLuint sceneColourTex;
	GLuint sceneDepthTex;

	std::vector<GLuint> processFBOs;
	std::vector<GLuint> processColourTexes;

	GLuint texWidth;
	GLuint texHeight;

	GLuint finalProcessTex;
};

