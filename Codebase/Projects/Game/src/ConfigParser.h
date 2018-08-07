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
		string Normal;
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
	struct EnemyType
	{
		string Identifier;
		int Hitpoints;
		float Size;
		float Speed;
		string DestroyAnim;
		string DestroyPart;
		string Mesh;
		float Scale;
		float RotationX;
		float RotationY;
		float RotationZ;
		float TranslationX;
		float TranslationY;
		float TranslationZ;
	};

	// Contains info about a gun
	struct GunType
	{
		string Identifier;
		float ProjectileSpeed;
		float ParticleMass;
		float FireRate;
		int Damage;
		char Hotkey;
		string SpriteName;
		float SpriteRadius;
		float TranslationX;
		float TranslationY;
		float TranslationZ;
	};

	// Contains a sprite texture info
	struct SpriteTexture
	{
		string Identifier;
		string FilePath;
	};

	// Descripes a sprite animation
	struct Animation
	{
		string Identifier;
		string SpriteName;
		float SpriteRadius;
		float Duration;
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
	struct SpawnInfo
	{
		float Interval;
		float MinHeight;
		float MaxHeight;
		float OuterCircleRadius;
		float InnerCircleRadius;
		float RemoveCircleRadius;
	};

#pragma region Properties

	map<string, SpriteTexture> GetSpriteTextures();
	map<string, Animation> GetAnimations();
	map<string, EnemyType> GetEnemyTypes();
	vector<RenderObject> GetRenderObjs();
	map<string, GunType> GetGunTypes();
	map<string, Mesh> GetMeshes();
	TerrainInfo GetTerrainInfo();
	SpawnInfo GetSpawnInfo();
	string GetResourceFolder();
	string GetShaderFolder();

#pragma endregion

	void Load(string filename);

	~ConfigParser();

private:

	map<string, SpriteTexture> spriteDictionary;
	map<string, Animation> animationDictionary;
	map<string, EnemyType> enemyDictionary;
	map<string, GunType> gunDictionary;
	map<string, Mesh> meshDictionary;
	vector<RenderObject> renderObjs;
	TerrainInfo terrainInfo;
	SpawnInfo spawnInfo;
	string resourceFolder;
	string shaderFolder;
};