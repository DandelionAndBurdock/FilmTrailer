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

Shader* ShaderManager::GetShader(std::string name) const {
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


ShaderManager* ShaderManager::GetInstance() {
	static ShaderManager manager;
	return &manager;
}