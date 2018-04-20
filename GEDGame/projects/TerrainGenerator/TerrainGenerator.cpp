#pragma once

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <random>
#include <time.h>
#include <vector>
#include <string.h>
/*
GED Utils
*/
#include "SimpleImage.h"
#include "TextureGenerator.h"
/*
Sonstiges
*/
#include "DiamondSquare.h"

/*
Arrayzugriff (Breite w) an Position X/Y
*/
#define IDX(x, y, w) ((x) + (y) * (w))

using namespace std;

/*
Kopiert ein Array komplett auf ein anderes
*/
void copyArray(const float * pi_dSrc, float * pi_dDest, const int pi_iWidth, const int pi_iHeight)
{
	for (int i = 0; i < pi_iWidth; i++)
	{
		for (int j = 0; j < pi_iHeight; j++)
		{
			pi_dDest[IDX(i, j, pi_iWidth)] = pi_dSrc[IDX(i, j, pi_iWidth)];
		}
	}
}

/*
Glättet ein Eingabearray mit einem 3x3 Filter
*/
void smoothArray(float * pi_dArray, const int pi_iWidth, const int pi_iHeight)
{
	/*
	Erstelle neues Array
	*/
	float *l_dTemp = new float[pi_iWidth * pi_iHeight];
	/*
	Glätte Array
	*/
	for (int i = 0; i < pi_iWidth; i++)
	{
		for (int j = 0; j < pi_iHeight; j++)
		{
			/*
			Grenzwerte
			*/
			int l_iStartX = i == 0 ? 0 : i - 1;
			int l_iStartY = j == 0 ? 0 : j - 1;
			int l_iEndX = i == pi_iWidth - 1 ? pi_iWidth - 1 : i + 1;
			int l_iEndY = j == pi_iHeight - 1 ? pi_iHeight - 1 : j + 1;
			float l_dMean = 0.0F;
			/*
			Bestimme Mittelwert
			*/
			for (int x = l_iStartX; x <= l_iEndX; x++)
			{
				for (int y = l_iStartY; y <= l_iEndY; y++)
				{
					l_dMean += pi_dArray[IDX(x, y, pi_iWidth)];
				}
			}
			/*
			Speichere Mittelwert
			*/
			l_dTemp[IDX(i, j, pi_iWidth)] = l_dMean / ((l_iEndX - l_iStartX + 1) * (l_iEndY - l_iStartY + 1));
		}
	}
	/*
	Kopiere geglättetes Array auf das Eingabearray
	*/
	copyArray(l_dTemp, pi_dArray, pi_iWidth, pi_iHeight);
	/*
	Lösche das temp Array
	*/
	delete[] l_dTemp;
}

int main(int argc, char* argv[])
{
	int l_iResolution;
	/*
	Überprüfung ob Argumentzahl stimmt sowie ob Auflösung ein Integer ist
	*/
	if (argc != 9)
	{
		cerr << "Incorrect number of arguments!" << endl;
		system("pause");
		exit(1);
	}
	else if ((l_iResolution = atoi(argv[2])) <= 0)
	{
		cerr << "Resolution is zero or not valid!" << endl;
		system("pause");
		exit(1);
	}
	/*
	Heightfield
	*/
	float *l_dHeightField = new float[l_iResolution * l_iResolution];
	/*
	Initialisiere Generator
	*/
	normal_distribution<float> l_Random(0.5F, 0.5F);
	default_random_engine l_Generator;
	srand(time(nullptr));
	/*
	Fülle Heightfield
	*/
	for (int i = 0; i < l_iResolution * l_iResolution; i++) 
	{
		/*
		Bestimme zufällige Zahl (bis gültig) und speichere im Array
		*/
		float l_dHeight;
		while ((l_dHeight = l_Random(l_Generator)) < 0.0F || l_dHeight > 1.0F) {}
		l_dHeightField[i] = l_dHeight;
	}	
	/*
	Erstelle Rechner mit vorgegebener Auflösung
	*/
	DiamondSquare l_dSquare (l_iResolution);
	/*
	Berechne HeightField
	*/
	l_dSquare.Compute();
	/*
	Speichere..
	*/
	float *l_dHeightField_DS = l_dSquare.GetHeightField();
	/*
	..und glätte
	*/
	smoothArray(l_dHeightField_DS, l_iResolution, l_iResolution);
	/*
	Leeres Bild
	*/
	GEDUtils::SimpleImage l_bmpHeightfield(l_iResolution, l_iResolution);
	vector<float> l_vecHeightfield = {};
	/*
	Fülle Bild
	*/
	for(int x = 0; x < l_iResolution; x++)
	{
		for (int y = 0; y < l_iResolution; y++)
		{
			l_bmpHeightfield.setPixel(x, y, l_dHeightField_DS[IDX(x, y, l_iResolution)]);
			l_vecHeightfield.push_back(l_dHeightField_DS[IDX(x, y, l_iResolution)]);
		}
	}
	/*
	Speichere Bild
	*/
	l_bmpHeightfield.save(argv[4]);
	/*
	TextureGenerator erstellen mit entsprechenden Pfaden
	*/
	GEDUtils::TextureGenerator l_TextureGen (L"gras.jpg",L"mud.jpg",L"pebble.jpg",L"rock.jpg");
	/*
	Wandle Argumente in WStrings um
	*/
	wstringstream wss;
	wstring arg6, arg8;
	wss << argv[6] << " " << argv[8];
	wss >> arg6;
	wss >> arg8;
	/*
	Generiere Texturen
	*/
	l_TextureGen.generateAndStoreImages(l_vecHeightfield, l_iResolution - 1, arg6, arg8);
	/*
	Aufräumen
	*/
	delete[] l_dHeightField;
	/*
	Ende
	*/
	system("pause");
    return 0;
}