// Class: Interpreter
// Description: Executes the code of 
// and input strategy string
// Author: Philip Jones
// Date: 17/10/2017

#pragma once

#define RNG RandomNumberGenerator::Instance()
#include "../glm/vec3.hpp"

namespace Random {
	// Some random colours
	const int NUM_RANDOM_COLOURS = 10;
	const glm::vec3 RANDOM_COLOURS[NUM_RANDOM_COLOURS] = {
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.411765f, 0.705882f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(0.627451f, 0.12549f, 0.941176f),
		glm::vec3(1.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.54902f, 0.0f),
		glm::vec3(0.0f, 0.74902f, 1.0f),
		glm::vec3(0.862745f, 0.0784314f, 0.235294f)
		 
	};
}

class RandomNumberGenerator
{

public:
	static RandomNumberGenerator* Instance();
	// TODO: Will this interval be closed or open?
	// Returns a random float in the closed interval [min, mix] 
	float GetRandFloat(float min = 0.0f, float max = 1.0f);
	// Returns a random int in the closed interval [min, mix] 
	int GetRandInt(int min, int max);
	// Get a random int in the range [min, max) which is not the excluded int
	int GetExcludedRandInt(int min, int max, int excluded);
	// Returns true or false with 50:50 chance
	bool TrueOrFalse();
	// Returns random number between min and max or -min and -max
	float GetRandOffset(float min = 0.0f, float max = 1.0f);
	// Returns a vector with 3 random components between 0 and 1.0
	glm::vec3 GetRandomVector();
	// Returns a random colour from RANDOM_COLOURS
	glm::vec3 GetRandomColour();
private:
	RandomNumberGenerator();
	~RandomNumberGenerator();
};
