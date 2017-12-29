#ifndef SHADERAI_H
#define SHADERAI_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>

class ShaderAI
{
public:
	GLuint Program;
	ShaderAI(const GLchar *vertexPath, const GLchar* fragmentPath);
	void Use();

private:
	const static int INFO_LOG_SIZE = 512;
};
#endif