#pragma once

#include <map>
#include <string>
#include <vector>

class Shader;

class ShaderManager
{
public:
	static ShaderManager* GetInstance();

	Shader* GetShader(std::string name) const;
	bool AddShader(const std::string& name, const std::string& vertex, const std::string& fragment, const std::string& geometry = "");
protected:
	ShaderManager();
	~ShaderManager();

	std::map<std::string, Shader*> shaders;  // Contains all Shaders loaded into the game mapped to a handle
};


