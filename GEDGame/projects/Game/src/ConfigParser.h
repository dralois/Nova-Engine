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

	// Saves enemy information
	struct EnemyType {
		string Identifier;
		int Hitpoints;
		float Size;
		float Speed;
		string Mesh;
		float Scale;
		float RotationX;
		float RotationY;
		float RotationZ;
		float TranslationX;
		float TranslationY;
		float TranslationZ;
	};

	// Stores terrain information
	struct TerrainInfo
	{
		float Width;
		float Height;
		float Depth;
		string HeightMap;
		string ColorMap;
		string NormalMap;
		float SpinSpeed;
	};

	// Stores spawn info
	struct SpawnInfo {
		float Interval;
		float MinHeight;
		float MaxHeight;
		float OuterCircleRadius;
		float InnerCircleRadius;
		float RemoveCircleRadius;
	};

#pragma region Properties

	map<string, EnemyType> GetEnemyTypes();
	vector<RenderObject> GetRenderObjs();
	map<string, Mesh> GetMeshes();
	TerrainInfo GetTerrainInfo();
	SpawnInfo GetSpawnInfo();

#pragma endregion

	void Load(string filename);

	~ConfigParser();

private:

	map<string, EnemyType> enemyDictionary;
	map<string, Mesh> meshDictionary;
	vector<RenderObject> renderObjs;
	TerrainInfo terrainInfo;
	SpawnInfo spawnInfo;
};