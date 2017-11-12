#pragma once

#include <map>
#include <string>

class Texture;

#define TEXTURE_MANAGER TextureManager::Instance()

class TextureManager
{
public:
	static TextureManager* Instance();

	bool BindTexture(const std::string& name);
	bool BindTexture1D(const std::string& name);
	Texture* GetTexture(const std::string& name) const;
	bool AddTexture(const std::string& name, const std::string& path);
	bool AddTexture(const std::string& name, float* data, int dimension); //TODO: Accept format as parameter
	// Generates a random 1D texture of 3-dimensional vector can be useful for sampling in shaders
	bool GenerateRandomTexture(const std::string name, int dimension);

protected:
	TextureManager();
	~TextureManager();

	std::map<std::string, Texture*> textures;  // Contains all textures loaded into the game mapped to a handle
};

