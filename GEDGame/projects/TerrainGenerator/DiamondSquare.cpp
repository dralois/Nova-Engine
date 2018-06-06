#include "DiamondSquare.h"

using namespace std;

#pragma region Properties

float *&DiamondSquare::GetHeightField()
{
	return m_dHeightField;
}

#pragma endregion

#pragma region Procedures

#pragma region Public

// Copy one array into another
void CopyArray(const float * pi_dSrc, float * pi_dDest, const int pi_iWidth, const int pi_iHeight)
{
	for (int i = 0; i < pi_iWidth; i++)
	{
		for (int j = 0; j < pi_iHeight; j++)
		{
			pi_dDest[IDX(i, j, pi_iWidth)] = pi_dSrc[IDX(i, j, pi_iWidth)];
		}
	}
}

// Smooth an array with a certain core radius
void SmoothArray(float * pi_dArray, const int &pi_iFilterRadius,
				const int &pi_iWidth, const int &pi_iHeight)
{
	float *l_dTemp = new float[pi_iWidth * pi_iHeight];
	// Go through array
	for (int i = 0; i < pi_iWidth; i++)
	{
		for (int j = 0; j < pi_iHeight; j++)
		{
			// Determine Bounds
			int l_iStartX = i == 0 ? 0 : i - pi_iFilterRadius;
			int l_iStartY = j == 0 ? 0 : j - pi_iFilterRadius;
			int l_iEndX = i == pi_iWidth - 1? pi_iWidth - 1 : i + pi_iFilterRadius;
			int l_iEndY = j == pi_iHeight - 1 ? pi_iHeight - 1 : j + pi_iFilterRadius;
			float l_dAvg = 0.0F;
			// Calculate avg
			for (int x = l_iStartX; x <= l_iEndX; x++)
			{
				for (int y = l_iStartY; y <= l_iEndY; y++)
				{
					l_dAvg += pi_dArray[IDX(x, y, pi_iWidth)];
				}
			}
			// Save avg
			l_dTemp[IDX(i, j, pi_iWidth)] = l_dAvg / ((l_iEndX - l_iStartX + 1) * (l_iEndY - l_iStartY + 1));
		}
	}
	// Copy temp into input array
	CopyArray(l_dTemp, pi_dArray, pi_iWidth, pi_iHeight);
	// Cleanup
	delete[] l_dTemp;
}

void DiamondSquare::Compute(const int &pi_iSmoothCycles)
{
	X_Initialize();
	// Walk through field
	int l_iResDS = m_iResolution + 1;
	for (int s = l_iResDS / 2; s >= 1; s /= 2)
	{
		// Diamond step
		for (int y = s; y < l_iResDS; y += 2 * s)
		{
			for (int x = s; x < l_iResDS; x += 2 * s)
			{
				X_Diamond(x, y, s); m_dHeightField[IDX(x, y, l_iResDS)] += m_dSigma * X_Random(-0.5F, 0.5F);
			}
		}
		// Square step
		for (int y = s; y < l_iResDS; y += 2 * s)
		{
			for (int x = s; x < l_iResDS; x += 2 * s)
			{
				X_Square(x - s, y, s); m_dHeightField[IDX(x - s, y, l_iResDS)] += m_dSigma * X_Random(-0.5F, 0.5F);
				X_Square(x, y - s, s); m_dHeightField[IDX(x, y - s, l_iResDS)] += m_dSigma * X_Random(-0.5F, 0.5F);
				X_Square(x + s, y, s); m_dHeightField[IDX(x + s, y, l_iResDS)] += m_dSigma * X_Random(-0.5F, 0.5F);
				X_Square(x, y + s, s); m_dHeightField[IDX(x, y + s, l_iResDS)] += m_dSigma * X_Random(-0.5F, 0.5F);
			}
		}
		// Update sigma
		m_dSigma /= powf(2.0f, m_dRoughness);
	}
	// Smooth afterwards
	for (int i = 0; i <= pi_iSmoothCycles; i++) {
		SmoothArray(m_dHeightField, 1, l_iResDS, l_iResDS);
	}
	// Clamp to size
	X_Clamp();
}

string DiamondSquare::toString()
{
	stringstream ss;
	// Builds a string
	for (int x = 0; x < m_iResolution; x++)
	{
		for (int y = 0; y < m_iResolution; y++)
		{
			ss << m_dHeightField[IDX(x, y, m_iResolution)] << " ";
		}
		// Add new line
		ss << endl;
	}
	// Return
	return ss.str();
}

#pragma endregion

#pragma region Private

void DiamondSquare::X_Initialize()
{
	// Initialize
	m_dHeightField = new float[(m_iResolution + 1) * (m_iResolution + 1)];
	m_Randomizer = normal_distribution<float>(0.0F, 0.25F);
	m_Generator = default_random_engine();
	// Generator is using the same seed now every time
	m_Generator.seed(m_iSeed);	
	// Fill corners
	m_dHeightField[IDX(0, 0, m_iResolution + 1)] = X_Random(0.0F, 1.0F);
	m_dHeightField[IDX(0, m_iResolution, m_iResolution + 1)] = X_Random(0.0F, 1.0F);
	m_dHeightField[IDX(m_iResolution, 0, m_iResolution + 1)] = X_Random(0.0F, 1.0F);
	m_dHeightField[IDX(m_iResolution, m_iResolution, m_iResolution + 1)] = X_Random(0.0F, 1.0F);
}

void DiamondSquare::X_Clamp()
{
	float * l_dTemp = new float[m_iResolution * m_iResolution];
	// Save in temp array
	for(int x = 0; x < m_iResolution; x++)
	{
		for (int y = 0; y < m_iResolution; y++)
		{
			l_dTemp[IDX(x, y, m_iResolution)] = m_dHeightField[IDX(x, y, m_iResolution + 1)];
		}
	}
	// Shrink array
	delete[] m_dHeightField;
	m_dHeightField = new float[m_iResolution * m_iResolution];
	// Copy back and normalize values
	for (int x = 0; x < m_iResolution; x++)
	{
		for (int y = 0; y < m_iResolution; y++)
		{
			float l_dVal = l_dTemp[IDX(x, y, m_iResolution)];
			m_dHeightField[IDX(x, y, m_iResolution)] = l_dVal > 1.0F ? 1.0F : l_dVal < 0.0F ? 0.0F : l_dVal;
		}
	}
	// Cleanup
	delete[] l_dTemp;
}

void DiamondSquare::X_Diamond(const int &pi_iX, const int &pi_iY, const int &pi_iLevel)
{
	// Calculate val
	float l_dAvg = (m_dHeightField[IDX(pi_iX - pi_iLevel, pi_iY - pi_iLevel, m_iResolution + 1)] +
					m_dHeightField[IDX(pi_iX - pi_iLevel, pi_iY + pi_iLevel, m_iResolution + 1)] +
					m_dHeightField[IDX(pi_iX + pi_iLevel, pi_iY - pi_iLevel, m_iResolution + 1)] +
					m_dHeightField[IDX(pi_iX + pi_iLevel, pi_iY + pi_iLevel, m_iResolution + 1)] ) / 4.0F;
	// Save val
	m_dHeightField[IDX(pi_iX, pi_iY, m_iResolution + 1)] = l_dAvg;
}

void DiamondSquare::X_Square(const int &pi_iX, const int &pi_iY, const int &pi_iLevel)
{
	// Calculate val (edge case handling!)
	float l_dAvg = 0.0F;
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
	// Save val
	m_dHeightField[IDX(pi_iX, pi_iY, m_iResolution + 1)] = l_dAvg;
}

float DiamondSquare::X_Random(const float & pi_dMin, const float & pi_dMax)
{
	float l_dRand = 0.0F;
	// Clamp to [-0.5, 0.5]
	while((l_dRand = m_Randomizer(m_Generator)) < -0.5F || l_dRand > 0.5F);
	// Random val calculator
	return ((pi_dMin + pi_dMax) / 2.0F) + (l_dRand * (pi_dMax - pi_dMin));
}

#pragma endregion

#pragma endregion

#pragma region Constructor & Destructor

DiamondSquare::DiamondSquare(const int & pi_iResolution) : 
	m_iResolution(pi_iResolution)
{

}

DiamondSquare::~DiamondSquare()
{
	delete[] m_dHeightField;
}

#pragma endregion