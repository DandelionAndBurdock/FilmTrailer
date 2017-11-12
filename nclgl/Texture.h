#pragma once

#include <string>
#include "GL/glew.h"	// For openGL types




class Texture
{
public:
	// Constructs the texture at path
	Texture(std::string path);	

	// Constructs a
	Texture(float* data, int dimension, bool oneDimensional = false);//TODO: Add image format

	// Cleans up
	~Texture();

	// Returns the ID of the texture object
	GLuint GetID() const { return ID; }

	// Set the wrapping mode in S and T axes
	void SetWrapping(bool repeatingS = true, bool repeatingT = true);

	// Set filtering mode from min and mag filtering
	void SetFiltering(bool linearMin = true, bool linearMax = true);

	// Returns result of attempt to load texture
	bool WasLoaded() { return loadSuccess; }

	// Bind the texture to active texture unit
	void Bind() const;

	// Bind one-dimensional texture to active texture unit
	void Bind1D() const;

protected:
	// Loads texture at path
	void LoadTexture(std::string path);

	GLuint ID; // ID for the texture

	bool loadSuccess;			// True if texture was loaded successfully

};
