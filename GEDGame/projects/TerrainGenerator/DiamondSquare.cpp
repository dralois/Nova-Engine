#include "DiamondSquare.h"

#include <iostream>

using namespace std;

#pragma region Properties

float * DiamondSquare::GetHeightField()
{
	return m_dHeightField;
}

#pragma endregion

#pragma region Procedures

string DiamondSquare::toString()
{
	stringstream ss;
	/*
	Erstellt String
	*/
	for (int x = 0; x < m_iResolution; x++)
	{
		for (int y = 0; y < m_iResolution; y++)
		{
			ss << m_dHeightField[IDX(x, y, m_iResolution)] << " ";
		}
		/*
		Zeilenumbruch
		*/
		ss << endl;
	}
	/*
	Gebe String zurück
	*/
	return ss.str();
}

void DiamondSquare::X_Initialize()
{
	/*
	Initialisiere Array, Randomizer..
	*/
	m_dHeightField = new float[(m_iResolution + 1) * (m_iResolution + 1)];
	m_Randomizer = normal_distribution<float>(0.0F, 1.0F);
	m_Generator = default_random_engine();
	m_Generator.seed(time(nullptr));
	/*
	Befülle Eckpunkte
	*/
	m_dHeightField[IDX(0, 0, m_iResolution + 1)] = X_Random(0.0F, 1.0F);
	m_dHeightField[IDX(0, m_iResolution, m_iResolution + 1)] = X_Random(0.0F, 1.0F);
	m_dHeightField[IDX(m_iResolution, 0, m_iResolution + 1)] = X_Random(0.0F, 1.0F);
	m_dHeightField[IDX(m_iResolution, m_iResolution, m_iResolution + 1)] = X_Random(0.0F, 1.0F);
	/*
	Debug Ausgabe der Eckpunkte
	*/
	cout << m_dHeightField[IDX(0, 0, m_iResolution + 1)] << endl;
	cout << m_dHeightField[IDX(0, m_iResolution, m_iResolution + 1)] << endl;
	cout << m_dHeightField[IDX(m_iResolution, 0, m_iResolution + 1)] << endl;
	cout << m_dHeightField[IDX(m_iResolution, m_iResolution, m_iResolution + 1)] << endl;
}

void DiamondSquare::Compute()
{
	/*
	Init
	*/
	X_Initialize();
	/*
	Schreite ausgehend von der Mitte durchs Array
	*/
	int l_iR = m_iResolution + 1;
	for (int s = l_iR / 2; s >= 1; s /= 2)
	{
		/*
		Führe Diamond Step aus
		*/
		for (int y = s; y < m_iResolution; y += 2 * s) 
		{
			for (int x = s; x < m_iResolution; x += 2 * s)
			{
				X_Diamond(x, y, s); m_dHeightField[IDX(x, y, m_iResolution + 1)] += m_dSigma * X_Random(-0.5F, 0.5F);
				float ld = m_dHeightField[IDX(x, y, m_iResolution + 1)];
			}
		}
		/*
		Führe Square Steps aus
		*/
		for (int y = s; y < m_iResolution; y += 2 * s)
		{
			for (int x = s; x < m_iResolution; x += 2 * s)
			{
				X_Square(x - s, y, s); m_dHeightField[IDX(x - s, y, m_iResolution + 1)] += m_dSigma * X_Random(-0.5F, 0.5F);
				X_Square(x, y - s, s); m_dHeightField[IDX(x, y - s, m_iResolution + 1)] += m_dSigma * X_Random(-0.5F, 0.5F);
				X_Square(x + s, y, s); m_dHeightField[IDX(x + s, y, m_iResolution + 1)] += m_dSigma * X_Random(-0.5F, 0.5F);
				X_Square(x, y + s, s); m_dHeightField[IDX(x, y + s, m_iResolution + 1)] += m_dSigma * X_Random(-0.5F, 0.5F);
				float ld1 = m_dHeightField[IDX(x - s, y, m_iResolution + 1)];
				float ld2 = m_dHeightField[IDX(x, y - s, m_iResolution + 1)];
				float ld3 = m_dHeightField[IDX(x + s, y, m_iResolution + 1)];
				float ld4 = m_dHeightField[IDX(x, y + s, m_iResolution + 1)];
			}
		}
		/*
		Aktualisiere Sigma
		*/
		m_dSigma /= powf(2.0f, m_dRoughness);
	}
	/*
	Auf richtige Größe zuschneiden
	*/
	X_Clamp();
}

void DiamondSquare::X_Clamp()
{
	float * l_dTemp = new float[m_iResolution * m_iResolution];
	/*
	"Kopiere" Werte in ein kleineres Array
	*/
	for(int x = 0; x < m_iResolution; x++)
	{
		for (int y = 0; y < m_iResolution; y++)
		{
			l_dTemp[IDX(x, y, m_iResolution)] = m_dHeightField[IDX(x, y, m_iResolution + 1)];
		}
	}
	/*
	Schrumpfe Array
	*/
	delete[] m_dHeightField;
	m_dHeightField = new float[m_iResolution * m_iResolution];
	/*
	Zurückkopieren und in Wertebereich [0-1] bringen
	*/
	for (int x = 0; x < m_iResolution; x++)
	{
		for (int y = 0; y < m_iResolution; y++)
		{
			float l_dVal = l_dTemp[IDX(x, y, m_iResolution)];
			m_dHeightField[IDX(x, y, m_iResolution)] = l_dVal > 1.0F ? 1.0F : l_dVal < 0.0F ? 0.0F : l_dVal;
		}
	}
	/*
	Aufräumen
	*/
	delete[] l_dTemp;
}

void DiamondSquare::X_Diamond(const int &pi_iX, const int &pi_iY, const int &pi_iLevel)
{
	/*
	Bestimme Wert
	*/
	float l_dAvg = (m_dHeightField[IDX(pi_iX - pi_iLevel, pi_iY - pi_iLevel, m_iResolution + 1)] +
					m_dHeightField[IDX(pi_iX - pi_iLevel, pi_iY + pi_iLevel, m_iResolution + 1)] +
					m_dHeightField[IDX(pi_iX + pi_iLevel, pi_iY - pi_iLevel, m_iResolution + 1)] +
					m_dHeightField[IDX(pi_iX + pi_iLevel, pi_iY + pi_iLevel, m_iResolution + 1)] ) / 4.0F;
	/*
	Setze Wert
	*/
	m_dHeightField[IDX(pi_iX, pi_iY, m_iResolution + 1)] = l_dAvg;
}

void DiamondSquare::X_Square(const int &pi_iX, const int &pi_iY, const int &pi_iLevel)
{
	/*
	Bestimme Wert (Beachte Randfälle)
	*/
	float l_dAvg;
	if (pi_iX == 0) {
		l_dAvg=(m_dHeightField[IDX(pi_iX, pi_iY - pi_iLevel, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX, pi_iY + pi_iLevel, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX + pi_iLevel, pi_iY, m_iResolution + 1)]) / 3.0F;
	}
	else if (pi_iX == m_iResolution) {
		l_dAvg=(m_dHeightField[IDX(pi_iX, pi_iY - pi_iLevel, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX, pi_iY + pi_iLevel, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX - pi_iLevel, pi_iY, m_iResolution + 1)]) / 3.0F;
	}
	else if (pi_iY == 0) {
		l_dAvg=(m_dHeightField[IDX(pi_iX, pi_iY + pi_iLevel, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX + pi_iLevel, pi_iY, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX - pi_iLevel, pi_iY, m_iResolution + 1)]) / 3.0F;
	}
	else if (pi_iY == m_iResolution) {
		l_dAvg=(m_dHeightField[IDX(pi_iX, pi_iY - pi_iLevel, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX + pi_iLevel, pi_iY, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX - pi_iLevel, pi_iY, m_iResolution + 1)]) / 3.0F;
	}
	else {
		l_dAvg=(m_dHeightField[IDX(pi_iX, pi_iY - pi_iLevel, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX, pi_iY + pi_iLevel, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX + pi_iLevel, pi_iY, m_iResolution + 1)] +
				m_dHeightField[IDX(pi_iX - pi_iLevel, pi_iY, m_iResolution + 1)]) / 4.0F;
	}
	/*
	Setze Wert
	*/
	m_dHeightField[IDX(pi_iX, pi_iY, m_iResolution + 1)] = l_dAvg;
}

float DiamondSquare::X_Random(const float & pi_dMin, const float & pi_dMax)
{
	float l_dRand;
	/*
	Auf -0.5/0.5 zuschneiden
	*/
	while((l_dRand = m_Randomizer(m_Generator)) < -0.5F || l_dRand > 0.5F);
	/*
	Zufallszahl zwischen Min/Max zurückgeben
	*/
	return (pi_dMax / 2.0F) - (l_dRand * (pi_dMax - pi_dMin));
}

#pragma endregion

#pragma region Constructor & Destructor

DiamondSquare::DiamondSquare(const int & pi_iResolution)
{
	m_iResolution = pi_iResolution;
}

DiamondSquare::~DiamondSquare()
{
	delete[] m_dHeightField;
}

#pragma endregion