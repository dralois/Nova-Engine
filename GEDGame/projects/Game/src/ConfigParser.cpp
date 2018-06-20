#include "ConfigParser.h"

#pragma region Properties

vector<ConfigParser::RenderObject> ConfigParser::GetRenderObjs()
{
	return renderObjs;
}

map<string, ConfigParser::Mesh> ConfigParser::GetMeshes()
{
	return meshDictionary;
}

ConfigParser::SpawnInfo ConfigParser::GetSpawnInfo()
{
	return spawnInfo;
}

map<string, ConfigParser::EnemyType> ConfigParser::GetEnemyTypes()
{
	return enemyDictionary;
}

ConfigParser::TerrainInfo ConfigParser::GetTerrainInfo()
{
	return terrainInfo;
}

#pragma endregion

#pragma region Procedures

void ConfigParser::Load(string filename)
{
	// Open file stream for reading
	ifstream l_strSrc(filename.c_str(), ifstream::in);
	// If stream open
	if (l_strSrc.is_open())
	{
		string name;
		// While end of file isnt reached	
		while (!l_strSrc.eof()) {			
			// Read next line	
			l_strSrc >> name;
			// Depending on variable safe to parser		
			if (name.empty() || (name[0] == '#')) {
				getline(l_strSrc, name);
				continue;
			}
			else if (name == "Mesh")
			{
				l_strSrc >> name;
				meshDictionary[name].Identifier = name;
				l_strSrc >> meshDictionary[name].File >> meshDictionary[name].Diffuse >> 
					meshDictionary[name].Specular >> meshDictionary[name].Glow;
			}
			else if (name == "RenderObject")
			{
				RenderObject newObj;
				l_strSrc >> newObj.Classification >> newObj.Identifier >> newObj.Scale >> newObj.RotationX >> newObj.RotationY >> 
					newObj.RotationZ >> newObj.TranslationX >> newObj.TranslationY >> newObj.TranslationZ;
				renderObjs.push_back(newObj);
			}
			else if (name == "EnemyType")
			{
				l_strSrc >> name;
				enemyDictionary[name].Identifier = name;
				l_strSrc >> enemyDictionary[name].Hitpoints >> enemyDictionary[name].Size >> enemyDictionary[name].Speed >>
					enemyDictionary[name].Mesh >> enemyDictionary[name].Scale >> enemyDictionary[name].RotationX >>
					enemyDictionary[name].RotationY >> enemyDictionary[name].RotationZ >> enemyDictionary[name].TranslationX >>
					enemyDictionary[name].TranslationY >> enemyDictionary[name].TranslationZ;
			}
			else if (name == "Spawn")
			{
				l_strSrc >> spawnInfo.Interval >> spawnInfo.MinHeight >> spawnInfo.MaxHeight >>
					spawnInfo.OuterCircleRadius >> spawnInfo.InnerCircleRadius >> spawnInfo.RemoveCircleRadius;
			}
			else if (name == "Terrain")
			{
				l_strSrc >> terrainInfo.Width >> terrainInfo.Height >> terrainInfo.Depth >>
					terrainInfo.HeightMap >> terrainInfo.ColorMap >> terrainInfo.NormalMap >> terrainInfo.SpinSpeed;
			}
			else
			{				
				// Parameter not recognized
				cerr << "Unknown parameter " << name << endl;
			}
			if (l_strSrc.fail() && !l_strSrc.eof()) {
				// Reading error
				cerr << "Failed reading parameter " << name << endl;
			}
		}		
		// Cleanup
		l_strSrc.close();
	}
}

#pragma endregion

#pragma region Constructors & Destructors

ConfigParser::ConfigParser(){ }

ConfigParser::~ConfigParser(){ }

#pragma endregion