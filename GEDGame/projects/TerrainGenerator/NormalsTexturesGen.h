#pragma once

#include <cstdlib>
#include <string>

#include "SimpleImage.h"

#define IDX(x, y, w) ((x) + (y) * (w))

// Vector X/Y/Z
struct Vec3 
{
	float x;
	float y;
	float z;

	// Multiply with a float
	Vec3 operator *(const float &pi_dVal) const
	{
		return Vec3(this->x * pi_dVal, this->y * pi_dVal, this->z * pi_dVal);
	}

	// Add a vector
	Vec3 operator +(const Vec3 &pi_v3Val) const 
	{
		return Vec3(this->x + pi_v3Val.x, this->y + pi_v3Val.y, this->z + pi_v3Val.z);
	}

	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vec3() : x(0), y(0), z(0) {}
};

class NormalsTexturesGen
{
public:
	
	NormalsTexturesGen(	const std::string &pi_sLowFlat, const std::string &pi_sLowSteep, 
						const std::string &pi_sHighFlat, const std::string &pi_sHighSteep);
	~NormalsTexturesGen();

	// Generates textures based on a heightfield
	void GenerateAndStore(	float * &pi_dHeightField, const int &pi_iResolution, 
							const std::string &pi_sColorsPath, const std::string &pi_sNormalsPath);
private:
	
	GEDUtils::SimpleImage m_bmpLowFlat;
	GEDUtils::SimpleImage m_bmpLowSteep;
	GEDUtils::SimpleImage m_bmpHighFlat;
	GEDUtils::SimpleImage m_bmpHighSteep;
	
	float * m_dHeightField;
	Vec3 * m_v3NormalField;
	int m_iResolution;

	// Create normal map
	void X_GenerateNormals(const std::string &pi_sNormalsPath);

	// Create color texture
	void X_GenerateColors(const std::string &pi_sColorsPath);
};