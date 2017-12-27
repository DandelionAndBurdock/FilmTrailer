#pragma once

class Mesh;
class Shader;

#include <GL/glew.h>
#include <string>

class ShaderArt
{
public:
	ShaderArt(GLfloat screenWidth, GLfloat screenHeight, const std::string& vs, const std::string& fs);
	~ShaderArt();

	void Draw();
	void Update(GLfloat timeSec);
protected:
	// Quad to draw the art on
	Mesh* quad;
	// Shader to generate art
	Shader* shader;

protected:
	// Screen Dimensions
	GLfloat screenWidth;
	GLfloat screenHeight;

	// Time in seconds
	GLfloat time = 0.0f;
};

