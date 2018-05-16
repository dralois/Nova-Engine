#include "NormalsTexturesGen.h"

using namespace std;
using namespace GEDUtils;

#pragma region Procedures

#pragma region Public

void NormalsTexturesGen::GenerateAndStore(	float *&pi_dHeightField, const int &pi_iResolution, 
											const string &pi_sColorsPath, const string &pi_sNormalsPath)
{
	m_iResolution = pi_iResolution;
	m_dHeightField = pi_dHeightField;
	m_v3NormalField = new Vec3[m_iResolution * m_iResolution];
	X_GenerateNormals(pi_sNormalsPath);
	X_GenerateColors(pi_sColorsPath);
}

#pragma endregion

#pragma region Private

#pragma region Helper Functions

/*
Überschneidet vier Farben und die zugehörigen Alphawerte (Alpha 0 ist immer 1)
*/
inline Vec3 X_GetBlendedCol(const Vec3 &pi_dColor0, const Vec3 &pi_dColor1, const Vec3 &pi_dColor2, const Vec3 &pi_dColor3, 
							const float &pi_dAlpha1, const float &pi_dAlpha2, const float &pi_dAlpha3)
{
	return pi_dColor3 * pi_dAlpha3 + (pi_dColor2 * pi_dAlpha2 + 
		(pi_dColor1 * pi_dAlpha1 + pi_dColor0 * (1 - pi_dAlpha1)) * (1 - pi_dAlpha2)) * (1 - pi_dAlpha3);
}

/*
Kalkuliere Alpha-Werte basierend auf Höhe und Steigung
*/
inline void X_CalcAlphas(float pi_dHeight, float pi_dSlope, float &po_dAlpha1, float &po_dAlpha2, float &po_dAlpha3)
{
	po_dAlpha1 = (1.0F - pi_dHeight) * (pi_dSlope > 0.3 ? 1.0F : 0.05F);
	po_dAlpha2 = pi_dHeight * (pi_dSlope <= 0.3 ? 1.0F : 0.05F);
	po_dAlpha3 = pi_dHeight * (pi_dSlope > 0.3 ? 1.0F : 0.05F);
}

/*
Farbe als Vektor
*/
inline Vec3 X_GetColorTiled(const SimpleImage &pi_bmpImage, UINT pi_iX, UINT pi_iY)
{
	Vec3 l_v3Return;
	pi_bmpImage.getPixel(pi_iX % pi_bmpImage.getWidth(), pi_iY % pi_bmpImage.getHeight(),
						l_v3Return.x, l_v3Return.y, l_v3Return.z);
	return l_v3Return;
}

/*
Vektorlänge
*/
inline float X_Length(const Vec3 &pi_v3Val)
{
	return sqrtf(powf(pi_v3Val.x, 2.0F) + powf(pi_v3Val.y, 2.0F) + powf(pi_v3Val.z, 2.0F));
}

/*
Normalisiere Vektor
*/
inline void X_Normalize(Vec3 &pi_v3Val)
{
	float l_dLength = X_Length(pi_v3Val);
	pi_v3Val.x /= l_dLength;
	pi_v3Val.y /= l_dLength;
	pi_v3Val.z /= l_dLength;
}

#pragma endregion

void NormalsTexturesGen::X_GenerateNormals(const string &pi_sNormalsPath)
{
	SimpleImage l_bmpNormals(m_iResolution, m_iResolution);
	/*
	Bestimme Normalen für alle Pixel
	*/
	for(int x = 0; x < m_iResolution; x++)
	{
		for (int y = 0; y < m_iResolution; y++)
		{
			/*
			Normalen X/Y berechnen
			*/
			float l_vTU = (m_dHeightField[IDX(x < m_iResolution - 1 ? x + 1 : x, y, m_iResolution)] - 
				m_dHeightField[IDX(x > 0 ? x - 1 : x, y, m_iResolution)]) / 2.0F * m_iResolution;
			float l_vTV = (m_dHeightField[IDX(x, y < m_iResolution - 1 ? y + 1 : y, m_iResolution)] - 
				m_dHeightField[IDX(x, y > 0 ? y - 1 : y, m_iResolution)]) / 2.0F * m_iResolution;
			/*
			Normale aufstellen
			*/
			Vec3 l_v3N(-l_vTU, -l_vTV, 1);
			/*
			Normalisieren
			*/
			X_Normalize(l_v3N);
			/*
			In den Bereich [0-1] bringen
			*/
			l_v3N.x = (l_v3N.x + 1.0F) / 2.0F;
			l_v3N.y = (l_v3N.y + 1.0F) / 2.0F;
			l_v3N.z = (l_v3N.z + 1.0F) / 2.0F;
			/*
			In dem Bild speichern
			*/
			l_bmpNormals.setPixel(x, y, l_v3N.x, l_v3N.y, l_v3N.z);
			/*
			Im Normalarray speichern
			*/
			m_v3NormalField[IDX(x, y, m_iResolution)] = l_v3N;
		}
	}
	/*
	Normalmap speichern
	*/
	l_bmpNormals.save(pi_sNormalsPath.c_str());
}

void NormalsTexturesGen::X_GenerateColors(const string &pi_sColorsPath)
{
	SimpleImage l_bmpColors(m_iResolution, m_iResolution);
	/*
	Bestimme überschnittene Farbwerte für alle Pixel
	*/
	for (int x = 0; x < m_iResolution; x++)
	{
		for (int y = 0; y < m_iResolution; y++)
		{
			/*
			Berechne Steigung und hole Höhe
			*/
			float l_dSlope = acosf(m_v3NormalField[IDX(x, y, m_iResolution)].z) / (3.1416F * 0.5F);	
			float l_dHeight = m_dHeightField[IDX(x, y, m_iResolution)];
			float l_dAlpha1, l_dAlpha2, l_dAlpha3;
			/*
			Berechne damit Alpha-Werte
			*/
			X_CalcAlphas(l_dHeight, l_dSlope, l_dAlpha1, l_dAlpha2, l_dAlpha3);
			/*
			Hole Farbwerte basierend auf X/Y
			*/
			Vec3 l_v3Color0 = X_GetColorTiled(m_bmpLowFlat, x, y);
			Vec3 l_v3Color1 = X_GetColorTiled(m_bmpLowSteep, x, y);
			Vec3 l_v3Color2 = X_GetColorTiled(m_bmpHighFlat, x, y);
			Vec3 l_v3Color3 = X_GetColorTiled(m_bmpHighSteep, x, y);
			/*
			Überschneide die Werte
			*/
			Vec3 l_v3Blended = X_GetBlendedCol(l_v3Color0, l_v3Color1, l_v3Color2, l_v3Color3, 
												l_dAlpha1, l_dAlpha2, l_dAlpha3);
			/*
			Speichere im Bild
			*/
			l_bmpColors.setPixel(x, y, l_v3Blended.x, l_v3Blended.y, l_v3Blended.z);
		}
	}
	/*
	Speichere das Bild
	*/
	l_bmpColors.save(pi_sColorsPath.c_str());
}

#pragma endregion

#pragma endregion

#pragma region Constructors & Destructors

NormalsTexturesGen::NormalsTexturesGen(	const string &pi_sLowFlat, const string &pi_sLowSteep,
										const string &pi_sHighFlat,	const string &pi_sHighSteep) :
	m_bmpLowFlat(pi_sLowFlat.c_str()),
	m_bmpLowSteep(pi_sLowSteep.c_str()),
	m_bmpHighFlat(pi_sHighFlat.c_str()),
	m_bmpHighSteep(pi_sHighSteep.c_str())
{

}

NormalsTexturesGen::~NormalsTexturesGen()
{
	delete[] m_v3NormalField;
}

#pragma endregion