// Mathmaticians forgive me
#pragma once

#include <tuple> 
#include <utility> // For pair
#include <vector>

#include "../../glm/vec3.hpp"
class CubicSpliner
{
public:
	// Initialise
	static std::vector<float> GetCubicCoefficients(std::vector<glm::vec3>& coordinates, std::vector<float>& timePoints);
	glm::vec3 GetPoint(float time);
protected:
	typedef  std::tuple<std::vector<float>, std::vector<float>, std::vector<float>> Diagonals;
	static  Diagonals BuildTriDiagonalMatrix(std::vector<float> timePoints);
	static  std::pair<std::vector<float>, std::vector<float>> LUDecomposition(std::vector<float>& lower, std::vector<float>& diag, 
		const std::vector<float>& upper);
	static  std::vector<float> ForwardSubstitution(std::vector<glm::vec3>& coordinates, const std::vector<float>& lower);
	static  std::vector<float> BackwardSubstitution(std::vector<glm::vec3>& coordinates, const std::vector<float>& diag, const std::vector<float>& upper);
	CubicSpliner() {}
	~CubicSpliner() {}
};

