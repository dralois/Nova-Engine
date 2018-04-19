#pragma once

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <vector>

using namespace std;

/*
Vergleichsfunktion
*/
bool CompareFct(int a, int b) { return a > b; }

int main()
{
	int l_iInput = -1;
	vector<int> l_vecIn = {};
	/*
	Hole Input und speichere im Vektor
	*/
	while (l_iInput != 0)
	{
		cin >> l_iInput;
		l_vecIn.push_back(l_iInput);
	}
	/*
	Sortiere Vektor
	*/
	sort(l_vecIn.begin(), l_vecIn.end(), CompareFct);
	/*
	Gebe aus
	*/
	for (auto &Itr : l_vecIn) 
	{
		cout << Itr << endl;
	}
	/*
	Ende
	*/
	system("pause");
    return 0;
}