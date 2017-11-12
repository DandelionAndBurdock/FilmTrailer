// Class: Interpreter
// Description: Executes the code of 
// and input strategy string
// Author: Philip Jones
// Date: 17/10/2017


#define RNG RandomNumberGenerator::Instance()

#pragma once
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
private:
	RandomNumberGenerator();
	~RandomNumberGenerator();
};
