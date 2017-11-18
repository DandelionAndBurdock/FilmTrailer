#include "CubicSpliner.h"



std::vector<float> CubicSpliner::GetCubicCoefficients(std::vector<glm::vec3> coordinates, std::vector<float> timePoints) {
	CubicSpliner::Diagonals d = BuildTriDiagonalMatrix(timePoints);
	std::pair<std::vector<float>, std::vector<float>> L = LUDecomposition(std::get<0>(d), std::get<1>(d), std::get<2>(d));



}

CubicSpliner::Diagonals CubicSpliner::BuildTriDiagonalMatrix(std::vector<float> timePoints) {
	std::vector<float> diagonals(timePoints.size(), 0.0f);
	std::vector<float> lowerDiagonals(timePoints.size(), 0.0f);
	std::vector<float> upperDiagonals(timePoints.size(), 0.0f);

	// Have coefficients with indexes 0, 1, ... n, setup a matrix to solve coefficients 1,...,n-1
	for (int i = 1; i <= timePoints.size() -1; ++i) {
		diagonals[i] = 2 * (timePoints[i] + timePoints[i + 1]);
		lowerDiagonals[i] = timePoints[i - 1];
		upperDiagonals[i] = timePoints[i];
	}
	// Correct elements rather than use if statements in for loop
	lowerDiagonals[1] = 0.0f;
	diagonals[timePoints.size() - 1] = timePoints[timePoints.size() - 1];
	upperDiagonals[timePoints.size() - 1] = 0.0f;

	return std::make_tuple(lowerDiagonals, diagonals, upperDiagonals);
}

 std::pair<std::vector<float>, std::vector<float>> CubicSpliner::LUDecomposition(std::vector<float>& lower, std::vector<float>& diag,
	const std::vector<float>& upper) {
	 diag[1] = diag[1]; // Diag one doesn't change
	 for (int i = 2; i < lower.size(); ++i) {
		 lower[i] = lower[i] / diag[i - 1];
		 diag[i] = diag[i] - lower[i] * upper[i - 1];
	 }
}