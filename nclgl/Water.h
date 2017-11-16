// ThinMatrix OpenGL video tutorial: https://www.youtube.com/watch?v=HusvGeEDU_U
#pragma once

#include "Mesh.h"

class Water : public Mesh
{
	const int REFLECTION_WIDTH = 320;
	const int REFLECTION_HEIGHT = 180;

	const int REFRACTION_WIDTH = 1280;
	const int REFRACTION_HEIGHT = 720;

public:
	Water(GLint screenWidth, GLint screenHeight);
	~Water();

	void BindReflectionFramebuffer();
	void BindRefractionFramebuffer();
	void UnbindFramebuffer();
	GLuint GetReflectionTex() { return reflectionColourTexFBO; }
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
	GLuint reflectionDepthTexRBO; //TODO: Move to texture manager


	GLint displayWidth;
	GLint displayHeight;
};