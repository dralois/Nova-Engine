#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

class ConfigParser
{
public:

	ConfigParser();
	
	struct TerrainPath 
	{
		string Height;
		string Color;
		string Normal;
	};

	struct Color
	{
		float r;
		float g;
		float b;

		/*
		Gibt Farbe als String aus
		*/
		string toString() 
		{
			ostringstream ss;
			ss << "(" << r << "," << g << "," << b << ")";
			return ss.str();
		}
	};

#pragma region Properties

	float GetSpinning();
	float GetSpinSpeed();
	Color GetBackgroundColor();
	TerrainPath GetTerrainPath();
	float GetTerrainWidth();
	float GetTerrainDepth();
	float GetTerrainHeight();

#pragma endregion

	void Load(string filename);

	~ConfigParser();

private:

	float spinning;
	float spinSpeed;

	Color backgroundColor;

	TerrainPath terrainPath;

	float terrainWidth;
	float terrainDepth;
	float terrainHeight;
};