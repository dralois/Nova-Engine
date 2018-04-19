#pragma once

#include <iostream>
#include <tchar.h>
#include <cstdlib>
#include <ctime>

using namespace std;

/*
Arrayzugriff (Breite w) an Position X/Y
*/
#define IDX(x, y, w) ((x) + (y) * (w))

/*
Gibt ein Array aus
*/
void printArray(const float * pi_dArray, const int pi_iWidth, const int pi_iHeight) 
{
	for(int i = 0; i < pi_iWidth; i++)
	{
		for (int j = 0; j < pi_iHeight; j++) 
		{
			cout << pi_dArray[IDX(i, j, pi_iWidth)] << " ";
		}
		cout << "\n";
	}
}

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

int main()
{
	const int l_iWidth = 10;
	const int l_iHeight = 10;

	float *l_dArray = new float[l_iHeight * l_iWidth];

	srand((int) time(nullptr));

	for (int i = 0; i < l_iWidth; i++)
	{
		for (int j = 0; j < l_iHeight; j++)
		{
			l_dArray[IDX(i, j, l_iWidth)] = (float)rand() / RAND_MAX;
		}
	}

	printArray(l_dArray, l_iWidth, l_iHeight);

	smoothArray(l_dArray, l_iWidth, l_iHeight);

	cout << "\n";

	printArray(l_dArray, l_iWidth, l_iHeight);

	delete[] l_dArray;

	system("pause");

    return 0;
}
