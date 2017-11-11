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
	Texture* GetTexture(const std::string& name) const;
	bool AddTexture(const std::string& name, const std::string& path);
	bool AddTexture(const std::string& name, float* data, int dimension); //TODO: Accept format as parameter

protected:
	TextureManager();
	~TextureManager();

	std::map<std::string, Texture*> textures;  // Contains all textures loaded into the game mapped to a handle
};

