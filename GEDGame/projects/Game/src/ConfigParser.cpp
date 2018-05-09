#include "ConfigParser.h"

#pragma region Properties

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
	/*
	Öffne Stream zur Datei (Lesen)
	*/
	ifstream l_strSrc(filename.c_str(), ifstream::in);
	/*
	Falls Stream offen
	*/
	if (l_strSrc.is_open())
	{
		bool wasEmpty;
		/*
		Solange etwas gelesen werden kann
		*/
		do
		{
			/*
			Nächstes Wort in Variable lesen
			*/
			string name;
			l_strSrc >> name;
			/*
			Speichere ob Streamende erreicht wurde
			*/
			wasEmpty = name.empty();
			/*
			Werte nur aus falls nicht leer
			*/
			if (!wasEmpty)
			{
				/*
				Je nach Variable speichern
				*/
				if(name == "Spinning")			
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
					/*
					Fehlerhafter Parameter
					*/
					cerr << "wrong parameter: " << name << endl;
				}		
			}
		} while (!wasEmpty);
		/*
		Stream wieder schließen
		*/
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