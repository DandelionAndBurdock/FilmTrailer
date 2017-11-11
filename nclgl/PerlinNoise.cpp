#include "PerlinNoise.h"

#include "../glm/gtc/noise.hpp"
#include "TextureManager.h"

PerlinNoise::PerlinNoise()
{
	dimensions = 64;
	noiseValues = new GLfloat[dimensions * dimensions];
	numOctaves = 6;
	amplitude = 1.0f;
	persistence = 0.3f;
}


PerlinNoise::~PerlinNoise()
{
	delete noiseValues;
}


void PerlinNoise::GenerateNoise() {
	for (unsigned int x = 0; x < dimensions; ++x) {
		for (unsigned int y = 0; y < dimensions; ++y) {
			// Initialise to zero
			noiseValues[x * dimensions + y] = 0.0f;

			// Values to be passed into the noise function
			float sampleX = float(x + 1) / dimensions;
			float sampleY = float(y + 1) / dimensions;

			// Keep track of max value so that we can nomalise noise values to 0-1 
			// at the end(then we can just multiply noiseValues by maximum height later)
			float maxValue = 0.0f;
			float frequency = baseFrequency;
			for (int octave = 0; octave < numOctaves; ++octave, frequency *= 2) {
				// Periodic in range 0 - frequency
				float sample = glm::perlin(glm::vec2(sampleX, sampleY) * frequency, glm::vec2(frequency));
				// Renormalise from range -1 to 1 to range 0 to 1
				sample = sample	* 0.5f + 0.5f;
				// Accumulate noise
				noiseValues[x * dimensions + y] += sample * amplitude;

				maxValue += amplitude;
				amplitude *= persistence;
			}
			// Normalise 
			noiseValues[x * dimensions + y] /= maxValue;
		}
	}
}

void PerlinNoise::GenerateTexture() {
	GenerateNoise();
	TEXTURE_MANAGER->AddTexture("Noise", noiseValues, dimensions);
}