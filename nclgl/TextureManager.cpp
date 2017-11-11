#include "TextureManager.h"

#include "Texture.h"


//TODO: Use a template
TextureManager::TextureManager()
{
}


TextureManager::~TextureManager()
{
	for (auto texture : textures) {
		delete texture.second;
	}
}

Texture* TextureManager::GetTexture(const std::string& name) const {
	if (textures.find(name) != textures.end()) {
		return textures.at(name);
	}
	else {
		return nullptr;
	}
}

bool TextureManager::AddTexture(const std::string& name, const std::string& path) {
	Texture* t = new Texture(path);
	textures[name] = t;
	return t->WasLoaded();
}


bool TextureManager::BindTexture(const std::string& name) {
	Texture* tex = GetTexture(name);
	if (tex) {
		tex->Bind();
		return true;
	}
	else {
		return false;
	}
}

TextureManager* TextureManager::Instance() {
	static TextureManager manager;
	return &manager;
}

bool TextureManager::AddTexture(const std::string& name, float* data, int dimension) { // TODO: Make const
	Texture* t = new Texture(data, dimension);
	textures[name] = t;
	return t->WasLoaded();
}
