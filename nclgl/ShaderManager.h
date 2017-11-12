#pragma once

#include <map>
#include <string>
#include <vector>

#include "../GLEW/include/GL/glew.h"
#include "../../glm/glm.hpp"

class Shader;

#define SHADER_MANAGER ShaderManager::Instance()

class ShaderManager
{
public:
	static ShaderManager* Instance();

	bool SetShader(const std::string& name) const;
	Shader* GetShader(const std::string& name) const;
	bool AddShader(const std::string& name, const std::string& vertex, const std::string& fragment, const std::string& geometry = "");
	bool AddShader(const std::string& name, Shader* shader);

	bool SetUniform(const std::string& shader, const std::string& uniform, GLint i);
	bool SetUniform(const std::string& shader, const std::string& uniform, GLfloat f);
	bool SetUniform(const std::string& shader, const std::string& uniform, const glm::mat4& mat);
	bool SetUniform(const std::string& shader, const std::string& uniform, const glm::vec3& vec);

	std::vector<std::string> GetUniformNames(const std::string& shader);

protected:
	ShaderManager();
	~ShaderManager();

	std::map<std::string, Shader*> shaders;  // Contains all Shaders loaded into the game mapped to a handle
};


