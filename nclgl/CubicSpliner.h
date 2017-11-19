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
	static std::vector<float> GetCubicCoefficients(std::vector<float>& coordinates, std::vector<float>& timePoints);
protected:
	typedef  std::tuple<std::vector<float>, std::vector<float>, std::vector<float>> Diagonals;
	static  Diagonals BuildTriDiagonalMatrix(std::vector<float> timePoints);
	static  std::pair<std::vector<float>, std::vector<float>> LUDecomposition(std::vector<float>& lower, std::vector<float>& diag, 
		const std::vector<float>& upper);
	static  void  ForwardSubstitution(std::vector<float>& coordinates, const std::vector<float>& lower);
	static  void  BackwardSubstitution(std::vector<float> coeffs, std::vector<float>& coordinates, const std::vector<float>& diag, const std::vector<float>& upper);
	static std::vector<float> BuildSolutionVector(std::vector<float>& coordinates, std::vector<float>& timePoints);
	CubicSpliner() {}
	~CubicSpliner() {}
};

