// ThinMatrix OpenGL video tutorial: https://www.youtube.com/watch?v=HusvGeEDU_U
#pragma once

#include "Mesh.h"

class Water : public Mesh
{
	const int REFLECTION_WIDTH = 320;
	const int REFLECTION_HEIGHT = 180;

	const int REFRACTION_WIDTH = 1280;
	const int REFRACTION_HEIGHT = 720;

	const float WATER_HEIGHT = 100.0f;

public:
	Water(GLint screenWidth, GLint screenHeight);
	~Water();

	float GetHeight() { return WATER_HEIGHT; }
	void BindReflectionFramebuffer();
	void BindRefractionFramebuffer();
	void UnbindFramebuffer();
	GLuint GetReflectionTex() { return reflectionColourTexFBO; }
	GLuint GetRefractionTex() { return refractionColourTexFBO; }

	void Update(GLfloat msec);
protected:
	void SetupQuad();
	void SetupFramebuffers();
	void SetupReflectionFrameBuffer();
	void SetupRefractionFrameBuffer();
	



	// Frame Buffer Object
	GLuint refractionFBO;
	GLuint refractionColourTexFBO; //TODO: Move to texture manager
	GLuint reflectionFBO;
	GLuint reflectionColourTexFBO; //TODO: Move to texture manager
	// TODO: Render Buffer Object
	GLuint refractionDepthTexFBO; //TODO: Move to texture manager
	GLuint reflectionDepthTexFBO; //TODO: Move to texture manager


	GLint displayWidth;
	GLint displayHeight;
};