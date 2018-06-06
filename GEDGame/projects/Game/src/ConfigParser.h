#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

class ConfigParser
{
public:

	ConfigParser();
	
	// Stores mesh information
	struct Mesh 
	{
		string Identifier;
		string File;
		string Diffuse;
		string Specular;
		string Glow;
	};

	// Saves transformation configurations for objects that need to be rendered
	struct RenderObject 
	{
		string Classification;
		string Identifier;
		float Scale;
		float RotationX;
		float RotationY;
		float RotationZ;
		float TranslationX;
		float TranslationY;
		float TranslationZ;
	};

	// Stores terrain information
	struct TerrainPath 
	{
		string Height;
		string Color;
		string Normal;
	};

	// Stores a color
	struct Color
	{
		float r;
		float g;
		float b;

		// Returns color as string
		string toString() 
		{
			ostringstream ss;
			ss << "(" << r << "," << g << "," << b << ")";
			return ss.str();
		}
	};

#pragma region Properties

	vector<RenderObject> GetRenderObjs();
	map<string, Mesh> GetMeshes();
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

	vector<RenderObject> renderObjs;

	map<string, Mesh> meshDictionary;

	Color backgroundColor;

	TerrainPath terrainPath;

	float terrainWidth;
	float terrainDepth;
	float terrainHeight;
};