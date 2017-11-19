#include "CubicSpliner.h"



std::vector<float> CubicSpliner::GetCubicCoefficients(std::vector<float>& coordinates, std::vector<float>& timePoints) {
	std::vector<float> quadraticCoefficients = std::vector<float>(coordinates.size());
	std::vector<float> quadraticCoefficients = std::vector<float>(coordinates.size());
	std::vector<float> quadraticCoefficients = std::vector<float>(coordinates.size());
	std::vector<float> quadraticCoefficients = std::vector<float>(coordinates.size());

	CubicSpliner::Diagonals d = BuildTriDiagonalMatrix(timePoints);
	// Split triangle matrix into lower part with implicit 1s on the diagonal (not stored) and upper diagonal matrix (only diagonal elements change)
	std::pair<std::vector<float>, std::vector<float>> LU = LUDecomposition(std::get<0>(d), std::get<1>(d), std::get<2>(d));
	// Build solution vector
	std::vector<float> r = BuildSolutionVector(coordinates, timePoints);
	// Now have LUx = r solve Ly = r by forward substitution...
	ForwardSubstitution(r, LU.first);
	//and then find x from Ux = y by backward substitution
	BackwardSubstitution(quadraticCoefficients, r, LU.second, std::get<2>(d));
	// Find quadratic coefficients

	// Find cubic coefficients

	// Find linear coefficients

	// Find constant coefficients
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
	 diag[1] = diag[1]; // Element (1, 1) doesn't change
	 for (int i = 2; i < lower.size(); ++i) {
		 lower[i] = lower[i] / diag[i - 1];
		 diag[i] = diag[i] - lower[i] * upper[i - 1];
	 }

	 return std::make_pair(lower, upper);
}


void CubicSpliner::ForwardSubstitution(std::vector<float>& r, const std::vector<float>& lower) {
	 // Element (1, 1) unchanged
	 for (int i = 2; i < lower.size(); ++i) {
		 r[i] -= lower[i] * r[i - 1];
	 }
}

void CubicSpliner::BackwardSubstitution(std::vector<float> coeffs, std::vector<float>& r, const std::vector<float>& diag, const std::vector<float>& upper) {
	 // Element (n, n) by hand
	 coeffs[diag.size() - 1] = r[diag.size() - 1] / diag[diag.size() - 1];
	 for (int i = diag.size() - 2; i > 0; --i) {
		 coeffs[i] = (r[i] - upper[i] * coeffs[i + 1]) / diag[i];
	 }
 }

 std::vector<float> CubicSpliner::BuildSolutionVector(std::vector<float>& coordinates, std::vector<float>& timePoints) {
	 std::vector<float> r;
	 for (int i = 1; i < coordinates.size(); ++i) {
		 float temp = (coordinates[i + 1] - coordinates[i]) / timePoints[i] - (coordinates[i] - coordinates[i - 1]) / timePoints[i - 1];
		 r.push_back(3.0f  *temp);
	 }
 }