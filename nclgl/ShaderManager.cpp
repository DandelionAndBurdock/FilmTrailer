#include "ShaderManager.h"

#include "Shader.h"
#include <iostream>

ShaderManager::ShaderManager()
{
}


ShaderManager::~ShaderManager()
{
	for (auto shader : shaders) {
		delete shader.second;
	}
}

Shader* ShaderManager::GetShader(const std::string& name) const {
	if (shaders.find(name) != shaders.end()) {
		return shaders.at(name);
	}
	else {
		std::cout << "Failed to find " << name << std::endl;
		return nullptr;
	}
}

bool ShaderManager::AddShader(const std::string& name, const std::string& vertex, const std::string& fragment, const std::string& geometry) {
	Shader* s = new Shader(vertex, fragment, geometry);
	shaders[name] = s;
	if (!s->WasLoaded()) {
		return false;
	}
	else {
		return s->LinkProgram();
	}
	return false;
}

bool ShaderManager::AddShader(const std::string& name, Shader* shader) {
	if (shader && shader->WasLoaded()) {
		shaders[name] = shader;
		return true;
	}
	else {
		return false;
	}
}


ShaderManager* ShaderManager::Instance() {
	static ShaderManager manager;
	return &manager;
}

bool ShaderManager::SetShader(const std::string& name) const {
	Shader* shader = GetShader(name);
	if (shader) {
		shader->Use();
		return true;
	}
	else {
		return false;
	}
}

bool ShaderManager::SetUniform(const std::string& shader, const std::string& uniform, GLint i) {
	Shader* s = GetShader(shader);
	if (s) {
		s->Use();
		return (s->SetUniform(uniform, i) != -1);
	}
	else {
		return false;
	}
}

bool ShaderManager::SetUniform(const std::string& shader, const std::string& uniform, GLfloat f) {
	Shader* s = GetShader(shader);
	if (s) {
		s->Use();
		return (s->SetUniform(uniform, f) != -1);
	}
	else {
		return false;
	}
}

bool ShaderManager::SetUniform(const std::string& shader, const std::string& uniform, const glm::mat4& mat) {
	Shader* s = GetShader(shader);
	if (s) {
		s->Use();
		return (s->SetUniform(uniform, mat) != -1);
	}
	else {
		return false;
	}
}

bool ShaderManager::SetUniform(const std::string& shader, const std::string& uniform, const glm::vec3& vec) {
	Shader* s = GetShader(shader);
	if (s) {
		s->Use();
		return (s->SetUniform(uniform, vec) != -1);
	}
	else {
		return false;
	}
}

std::vector<std::string> ShaderManager::GetUniformNames(const std::string& shader) {
	Shader* s = GetShader(shader);
	if (s) {
		s->Use();
		return s->GetUniformNames();
	}
	else {
		return std::vector<std::string>();
	}

}