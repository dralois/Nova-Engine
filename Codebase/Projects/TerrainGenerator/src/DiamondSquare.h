#pragma once

#include <sstream>
#include <cstdlib>
#include <random>
#include <ctime>

#define IDX(x, y, w) ((x) + (y) * (w))

class DiamondSquare
{
public:
	
	DiamondSquare(const int &pi_iResolution);
	
	~DiamondSquare();

	// Reference to the heightfield
	float *&GetHeightField();

	// Runs algorithm (smoothes x times)
	void Compute(const int &pi_iSmoothCycles);

	// Debug output
	std::string toString();

private:
	
	std::normal_distribution<float> m_Randomizer;
	std::default_random_engine m_Generator;
	const unsigned int m_iSeed = 1247016462;
	float m_dRoughness = 0.9F;
	float m_dSigma = 1.0F;
	float *m_dHeightField;
	int m_iResolution;

	// Initialize values
	void X_Initialize();
	
	// Clamp to a base of 2
	void X_Clamp();

	// Diamond step
	void X_Diamond(const int &pi_iX, const int &pi_iY, const int &pi_iLevel);
	
	// Square step
	void X_Square(const int &pi_iX, const int &pi_iY, const int &pi_iLevel);
	
	// Get random float
	float X_Random(const float &pi_dMin, const float &pi_dMax);
};