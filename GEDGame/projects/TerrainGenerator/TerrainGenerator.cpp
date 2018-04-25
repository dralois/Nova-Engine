#pragma once

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <random>
#include <vector>
#include <ctime>
/*
GED Utils
*/
#include "SimpleImage.h"
#include "TextureGenerator.h"
/*
Sonstiges
*/
#include "DiamondSquare.h"
#include "NormalsTexturesGen.h"

/*
Arrayzugriff (Breite w) an Position X/Y
*/
#define IDX(x, y, w) ((x) + (y) * (w))

using namespace std;

/*
Verkleinert ein Array auf 1/(pi_iFactor * pi_iFactor) der Größe
*/
void X_DownsizeArray(float * &pi_dArray, const int &pi_iResolution, const int &pi_iFactor)
{
	/*
	Kleineres Array
	*/
	float * l_dReturn = new float[(pi_iResolution / pi_iFactor) * (pi_iResolution / pi_iFactor)];
	/*
	Mittelwert berechnen und im neuen Array speichern
	*/
	for (int x = 0; x < (pi_iResolution / pi_iFactor); x++)
	{
		for (int y = 0; y < (pi_iResolution / pi_iFactor); y++)
		{
			float l_dAvg = 0.0F;
			/*
			4x4 Bereich aufsummieren
			*/
			for (int i = x * pi_iFactor; i < (x * pi_iFactor) + pi_iFactor; i++)
			{
				for (int j = y * pi_iFactor; j < (y * pi_iFactor) + pi_iFactor; j++)
				{
					l_dAvg += pi_dArray[IDX(i, j, pi_iResolution)];
				}
			}
			/*
			Mitteln und abspeichern
			*/
			l_dAvg /= (pi_iFactor * pi_iFactor) * 1.0F;
			l_dReturn[(IDX(x, y, pi_iResolution / pi_iFactor))] = l_dAvg;
		}
	}
	/*
	Cleanup altes Array und "Rückgabe"
	*/
	delete[] pi_dArray;
	pi_dArray = l_dReturn;
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
	float *l_dRandomHeightField = new float[l_iResolution * l_iResolution];
	/*
	Initialisiere Generator
	*/
	normal_distribution<float> l_Random(0.5F, 0.5F);
	default_random_engine l_Generator;
	srand(static_cast<unsigned int>(time(nullptr)));
	/*
	Ausgabe
	*/
	cout << "Computing random heightfield" << endl;
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
		l_dRandomHeightField[i] = l_dHeight;
	}	
	/*
	Erstelle Rechner mit vorgegebener Auflösung
	*/
	DiamondSquare l_dSquare (l_iResolution);
	/*
	Ausgabe
	*/
	cout << "Computing DS heightfield" << endl;
	/*
	Berechne HeightField
	*/
	l_dSquare.Compute(10);
	/*
	Speichere..
	*/
	float *&l_dDSquareHeightField = l_dSquare.GetHeightField();
	/*
	TextureGenerator erstellen mit entsprechenden Pfaden
	*/
	NormalsTexturesGen l_MyGen ("../../../../external/textures/gras15.jpg",
								"../../../../external/textures/ground02.jpg",
								"../../../../external/textures/pebble03.jpg",
								"../../../../external/textures/rock4.jpg");	
	/*
	Ausgabe
	*/
	cout << "Generating color and normal textures" << endl;
	/*
	Texturen erstellen
	*/
	l_MyGen.GenerateAndStore(l_dDSquareHeightField, l_iResolution, argv[6], argv[8]);
	/*
	Heightfield runterskalieren
	*/
	int l_iFactor = 4;  
	X_DownsizeArray(l_dDSquareHeightField, l_iResolution, l_iFactor);
	/*
	Leeres Bild
	*/
	GEDUtils::SimpleImage l_bmpHeightfield(l_iResolution / l_iFactor, l_iResolution / l_iFactor);
	/*
	Ausgabe
	*/
	cout << "Generating height texture" << endl;
	/*
	Fülle Bild
	*/
	for (int x = 0; x < (l_iResolution / l_iFactor); x++)
	{
		for (int y = 0; y < (l_iResolution / l_iFactor); y++)
		{
			l_bmpHeightfield.setPixel(x, y, l_dDSquareHeightField[IDX(x, y, (l_iResolution / l_iFactor))]);
		}
	}
	/*
	Speichere Bild
	*/
	l_bmpHeightfield.save(argv[4]);
	/*
	Aufräumen
	*/
	delete[] l_dRandomHeightField;
	/*
	Ausgabe
	*/
	cout << "Cleanup and textures done" << endl;
	/*
	Ende
	*/
	system("pause");
    return 0;
}