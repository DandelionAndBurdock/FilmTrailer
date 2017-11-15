#pragma once

#include "OGLRenderer.h"

#include <map>

#define SHADER_VERTEX   0
#define SHADER_FRAGMENT 1
#define SHADER_GEOMETRY 2

class Shader	{
public:
	Shader(std::string vertex, std::string fragment, std::string geometry = "");
	~Shader(void);

	void Use() const;
	GLuint  GetProgram() { return program; }
	bool	LinkProgram();

	bool WasLoaded() const { return !loadFailed; }
	GLint GetLocation(const std::string& name);
	GLint SetUniform(const std::string& name, GLint i);
	GLint SetUniform(const std::string& name, GLfloat f);
	GLint SetUniform(const std::string& name, const glm::mat4& mat);
	GLint SetUniform(const std::string& name, const glm::vec3& vec);

	std::vector<std::string> GetUniformNames();

protected:
	bool	LoadShaderFile(std::string from, std::string &into);
	GLuint	GenerateShader(std::string from, GLenum type);
	void	SetDefaultAttributes();
	void SetUniformLocations();
	void AddUniformLocation(const std::string& name);
	void AddArrayUniformLocation(const std::string& name);
	void AddDirLightUniformLocation();
	void AddPointLightUniformLocation();
	void AddSpotLightUniformLocation();


	GLuint objects[3];
	GLuint program;

	bool loadFailed;
	
	std::vector<std::string> shaderText; // Contains shader string in text format during loading
	std::map<std::string, GLuint> uniformLocations;// Contains a map of uniformLocations calculated by SetUniformLocations()
};

