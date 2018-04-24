#pragma once

#include <sstream>
#include <cstdlib>
#include <random>
#include <ctime>

/*
Arrayzugriff (Breite w) an Position X/Y
*/
#define IDX(x, y, w) ((x) + (y) * (w))

class DiamondSquare
{
public:
	/*
	Erstellt DiamondSquare Rechner
	@param pi_iResolution: Aufl�sung (2^n)
	*/
	DiamondSquare(const int &pi_iResolution);
	~DiamondSquare();

	/*
	Hole Pointer auf berechnete H�hen
	*/
	float *GetHeightField();

	/*
	F�hrt Algorithmus aus
	@param pi_iSmoothCycles: Anzahl der Gl�ttungen
	*/
	void Compute(const int &pi_iSmoothCycles);

	/*
	F�r Debug Zwecke
	*/
	std::string toString();

private:
	/*
	Deklarationen
	*/
	std::normal_distribution<float> m_Randomizer;
	std::default_random_engine m_Generator;
	float m_dRoughness = 0.9F;
	float m_dSigma = 1.0F;
	float *m_dHeightField;
	int m_iResolution;

	/*
	Initialisiert Array, Randomizer..
	*/
	void X_Initialize();
	
	/*
	Reduziert die Gr��e auf Resolution - 1
	*/
	void X_Clamp();

	/*
	Diamond Step
	@param pi_iX: X-Position des Steps
	@param pi_iY: Y-Position des Steps
	@param pi_iLevel: Aktuelle Stufe (Schrittweite)
	*/
	void X_Diamond(const int &pi_iX, const int &pi_iY, const int &pi_iLevel);
	
	/*
	Square Step
	@param pi_iX: X-Position des Steps
	@param pi_iY: Y-Position des Steps
	@param pi_iLevel: Aktuelle Stufe (Schrittweite)
	*/
	void X_Square(const int &pi_iX, const int &pi_iY, const int &pi_iLevel);
	
	/*
	Gibt normal verteilte Zufallszahl
	@param pi_dMin: Minimum
	@param pi_dMax: Maximum
	*/
	float X_Random(const float &pi_dMin, const float &pi_dMax);
};