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

string ConfigParser::GetTerrainPath()
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
				if(name == "spinning")			
				{
					l_strSrc >> spinning;
				}
				else if (name == "spinSpeed")
				{
					l_strSrc >> spinSpeed;
				}
				else if (name == "backgroundColor")
				{
					l_strSrc >> backgroundColor.r >> backgroundColor.g >> backgroundColor.b;
				}
				else if (name == "terrainPath")
				{
					l_strSrc >> terrainPath;
				}
				else if (name == "terrainWidth")
				{
					l_strSrc >> terrainWidth;
				}
				else if (name == "terrainDepth")
				{
					l_strSrc >> terrainDepth;
				}
				else if (name == "terrainHeight")
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