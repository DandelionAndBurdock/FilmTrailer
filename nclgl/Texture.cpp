#include "Texture.h"

#include "../SOIL/SOIL.h"

Texture::Texture(std::string path) {
	loadSuccess = false;
	LoadTexture(path);
	SetWrapping();
	SetFiltering();

}

Texture::~Texture() {
	glDeleteTextures(1, &ID);
}

void Texture::SetWrapping(bool repeatingS, bool repeatingT) {
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		repeatingS ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		repeatingT ? GL_REPEAT : GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetFiltering(bool linearMin, bool linearMax) {
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		linearMin ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		linearMax ? GL_LINEAR : GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}
#include <iostream>
// Bind the texture to active texture unit
void Texture::Bind() const {
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::LoadTexture(std::string path) {
	ID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	loadSuccess = bool(ID);
}