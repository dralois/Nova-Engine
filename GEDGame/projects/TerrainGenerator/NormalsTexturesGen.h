#pragma once

#include <cstdlib>
#include <string>
/*
GED Utils
*/
#include "SimpleImage.h"

/*
Arrayzugriff (Breite w) an Position X/Y
*/
#define IDX(x, y, w) ((x) + (y) * (w))

/*
Vektor mit X/Y/Z
*/
struct Vec3 
{
	float x;
	float y;
	float z;

	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vec3() : x(0), y(0), z(0) {}
};

class NormalsTexturesGen
{
public:
	/*
	Erstellt TextureGenerator
	@param pi_sLowFlat:		Flach und nicht steil
	@param pi_sLowSteep:	Flach steil
	@param pi_sHighFlat:	Hoch und nicht steil
	@param pi_sHighSteep:	Hoch und steil
	*/
	NormalsTexturesGen(	const std::string &pi_sLowFlat, const std::string &pi_sLowSteep, 
						const std::string &pi_sHighFlat, const std::string &pi_sHighSteep);
	~NormalsTexturesGen();

	/*
	Generiert und speichert Normals und Color Textures
	@param pi_dHeightField:	HeightField als Array
	@param pi_iResolution:	Auflösung des Arrays
	@param pi_sColorsPath:	Speicherpfad Colors
	@param pi_sNormalsPath:	Speicherpfad Normals
	*/
	void GenerateAndStore(	float * pi_dHeightField, const int &pi_iResolution, 
							const std::string &pi_sColorsPath, const std::string &pi_sNormalsPath);
private:
	/*
	Deklarationen
	*/
	std::string m_sLowFlat;
	std::string m_sLowSteep;
	std::string m_sHighFlat;
	std::string m_sHighSteep;
	float * m_dHeightField;
	int m_iResolution;

	/*
	Erstellt Normal Map
	@param pi_sNormalPath: Pfad an den gespeichert werden soll
	*/
	void X_GenerateNormals(const std::string &pi_sNormalsPath);

	/*
	Erstellt Textur
	@param pi_sColorPath: Pfad an den gespeichert werden soll
	*/
	void X_GenerateColors(const std::string &pi_sColorsPath);
};