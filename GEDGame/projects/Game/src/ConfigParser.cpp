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

float ConfigParser::GetSpinning()
{
	return spinning;
}

float ConfigParser::GetSpinSpeed()
{
	return spinSpeed;
}

ConfigParser::Color ConfigParser::GetBackgroundColor()
{
	return backgroundColor;
}

ConfigParser::TerrainPath ConfigParser::GetTerrainPath()
{
	return terrainPath;
}

float ConfigParser::GetTerrainWidth()
{
	return terrainWidth;
}

float ConfigParser::GetTerrainDepth()
{
	return terrainDepth;
}

float ConfigParser::GetTerrainHeight()
{
	return terrainHeight;
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
			else if(name == "Spinning")			
			{
				l_strSrc >> spinning;
			}
			else if (name == "SpinSpeed")
			{
				l_strSrc >> spinSpeed;
			}
			else if (name == "BackgroundColor")
			{
				l_strSrc >> backgroundColor.r >> backgroundColor.g >> backgroundColor.b;
			}
			else if (name == "Mesh") {
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
			else if (name == "TerrainPath")
			{
				l_strSrc >> terrainPath.Height >> terrainPath.Color >> terrainPath.Normal;
			}
			else if (name == "TerrainWidth")
			{
				l_strSrc >> terrainWidth;
			}
			else if (name == "TerrainDepth")
			{
				l_strSrc >> terrainDepth;
			}
			else if (name == "TerrainHeight")
			{
				l_strSrc >> terrainHeight;
			}
			else
			{				
				// Parameter not recognized
				cerr << "wrong parameter: " << name << endl;
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

ConfigParser::ConfigParser()
{
}

ConfigParser::~ConfigParser()
{
}

#pragma endregion