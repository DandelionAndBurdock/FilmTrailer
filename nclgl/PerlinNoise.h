#pragma once
#include "GL/glew.h"

class PerlinNoise
{
public:
	PerlinNoise();
	~PerlinNoise();

	void GenerateNoise();
	void GenerateTexture(); //TODO: Accept name
protected:
	GLint dimensions;
	GLfloat* noiseValues;

	GLint numOctaves;
	GLint baseFrequency;
	GLfloat baseAmplitude;
	// Higher the persistence the more contribution higher octaves will make to the noise
	// Should be between 0 and 1 in most situations we care about (I think)
	GLfloat persistence; 
};

