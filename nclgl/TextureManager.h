#pragma once

#include <map>
#include <string>

class Texture;

class TextureManager
{
public:
	static TextureManager* GetInstance();

	Texture* GetTexture(std::string name) const;
	bool AddTexture(std::string name, std::string path);
	bool AddTexture(std::string name, float* data, int dimension); //TODO: Accept format as parameter

protected:
	TextureManager();
	~TextureManager();

	std::map<std::string, Texture*> textures;  // Contains all textures loaded into the game mapped to a handle
};

