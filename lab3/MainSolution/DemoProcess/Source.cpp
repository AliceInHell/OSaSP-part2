#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <Windows.h>

using namespace std;

string localString = "working fine...";

void main()
{
	while (1)
	{
		cout << localString.c_str() << endl;
		_getch();
	}
}