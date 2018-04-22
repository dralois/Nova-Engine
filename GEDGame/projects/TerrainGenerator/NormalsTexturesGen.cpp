#include "NormalsTexturesGen.h"

using namespace std;

#pragma region Procedures

#pragma region Public

void NormalsTexturesGen::GenerateAndStore(	float *pi_dHeightField, const int &pi_iResolution, 
											const string &pi_sColorsPath, const string &pi_sNormalsPath)
{
	m_dHeightField = pi_dHeightField;
	m_iResolution = pi_iResolution;
	X_GenerateNormals(pi_sNormalsPath);
	X_GenerateColors(pi_sColorsPath);
}

#pragma endregion

#pragma region Private

#pragma region Helper Functions

/*
Vektorlänge
*/
float X_Length(const Vec3 &pi_v3Val)
{
	return sqrtf(powf(pi_v3Val.x, 2.0F) + powf(pi_v3Val.y, 2.0F) + powf(pi_v3Val.z, 2.0F));
}

/*
Normalisiere Vektor
*/
void X_Normalize(Vec3 &pi_v3Val)
{
	float l_dLength = X_Length(pi_v3Val);
	pi_v3Val.x /= l_dLength;
	pi_v3Val.y /= l_dLength;
	pi_v3Val.z /= l_dLength;
}

#pragma endregion

void NormalsTexturesGen::X_GenerateNormals(const string &pi_sNormalsPath)
{
	GEDUtils::SimpleImage l_bmpNormals(m_iResolution, m_iResolution);
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
			float l_vTU = (m_dHeightField[IDX(x < m_iResolution - 1 ? x + 1 : x, y, m_iResolution)] - m_dHeightField[IDX(x > 0 ? x - 1 : x, y, m_iResolution)]) / 2.0F * m_iResolution;
			float l_vTV = (m_dHeightField[IDX(x, y < m_iResolution - 1 ? y + 1 : y, m_iResolution)] - m_dHeightField[IDX(x, y > 0 ? y - 1 : y, m_iResolution)]) / 2.0F * m_iResolution;
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
			In dem Bild speichern (X/Y muss vertauscht werden!)
			*/
			l_bmpNormals.setPixel(y, x, l_v3N.y, l_v3N.x, l_v3N.z);
		}
	}
	/*
	Normalmap speichern
	*/
	l_bmpNormals.save(pi_sNormalsPath.c_str());
}

void NormalsTexturesGen::X_GenerateColors(const string &pi_sColorsPath)
{
	// ToDo
}

#pragma endregion

#pragma endregion

#pragma region Constructors & Destructors

NormalsTexturesGen::NormalsTexturesGen(	const string &pi_sLowFlat, const string &pi_sLowSteep,
										const string &pi_sHighFlat,	const string &pi_sHighSteep) :
	m_sLowFlat(pi_sLowFlat),
	m_sLowSteep(pi_sLowSteep),
	m_sHighFlat(pi_sHighFlat),
	m_sHighSteep(pi_sHighSteep) {}

NormalsTexturesGen::~NormalsTexturesGen()
{

}

#pragma endregion