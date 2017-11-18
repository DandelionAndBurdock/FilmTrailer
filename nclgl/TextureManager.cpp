#include "TextureManager.h"

#include "Texture.h"

#include "../../glm/vec3.hpp"

#include "RandomNumberGenerator.h"

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

bool TextureManager::BindTexture1D(const std::string& name) {
	Texture* tex = GetTexture(name);
	if (tex) {
		tex->Bind1D();
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


bool  TextureManager::GenerateRandomTexture(const std::string name, int dimension) {
	const int NUM_COMPONENTS = 3; // for R, G and B components
	GLfloat* data = new GLfloat[NUM_COMPONENTS * dimension];

	for (GLuint i = 0; i < NUM_COMPONENTS * dimension; i += NUM_COMPONENTS) {
		data[i] = RNG->GetRandFloat();
		data[i + 1] = RNG->GetRandFloat();
		data[i + 2] = RNG->GetRandFloat();
	}

  	Texture* t = new Texture(&data[0], dimension, true);
	textures[name] = t;

	delete[] data;
	return t->WasLoaded();
}


bool TextureManager::AddTexture(const std::string& name, int ID) {
	Texture* t = new Texture(ID);
	textures[name] = t;
	return t->WasLoaded();
}