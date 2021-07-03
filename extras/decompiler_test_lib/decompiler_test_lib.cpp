#include <Windows.h>
#include <iostream>
#include "decompiler_test_lib.h"

//functions
int Func1(int a) {
	return a * 2;
}

TestA fff(TestA& a) {
	a.a = rand();
	a.b = rand();
	return a;
}

//global vars
TestA g_a;
TestB g_b;
TestC g_c;

int Test_SimpleFunc(int a, int b) {
	if (a < 0.5) {
		a += 3;
	}
	return a + b;
}

int Test_StructsAndArray(TestA* a) {
	auto c = g_c;
	auto obj = fff(*c.b->a);

	g_c.b->arr2[rand() *
		2][rand() + 5].arr[rand()] = 1;
	*a = obj;
	return 1000;
}

int Test_Array() {
	int arr[2][3][4];
	for (int i = 0; i < 120; i++)
		arr[GetTickCount()][GetTickCount()][GetTickCount()] = 300;

	return arr[1][2][3];
}

int Test_Switch() {
	auto a = GetTickCount();
	auto b = GetTickCount();

	switch (b)
	{
	case 1:
		b *= 10 + a;
		break;
	case 2:
		b *= 15 + a * a;
		break;
	case 4:
		b ++;
	case 5:
	case 6:
		b *= 30 + (int)pow(a, 3);
		break;
	
	default: {
		b = 0;
	}
	}

	return b;
}

int Test_FloatingPoints() {
	float b = (float)GetTickCount();

	if (b >= 1) {
		b++;
	}
	if (b > 2) {
		b++;
	}
	if (b <= 3) {
		b++;
	}
	if (b < 4) {
		b++;
	}
	if (b == 5) {
		b++;
	}
	if (b != 6) {
		b++;
	}

	if (b == 10 && b == 20 || b == 30 && b == 40 && b == 50 || b == 60) {
		 b+= 2;
	}

	return (int)b;
}

int Test_Cycles() {
	float b = (float)GetTickCount();
	float c = (float)GetTickCount();

	while (b < 10) {
		b++;
		if (b == c) {
			b--;
			break;
		}
		if (b == c + 1) {
			while (b < 100) {
				b++;
			}
			continue;
		}
		if (b == c + 2)
			break;
		if (b == c + 10) {
			b--;
			continue;
		}
		if (b == c + 3) {
			b += 2;
		}
		b++;
	}

	/*while (b == 10 || (b == 20 && b == 30)) {
		b++;
		if (b == 100) {
			while (b < 500) {
				b += 2;
			}
		}


		b += Func1(10) + Func1(5);
	}*/

	return (int)b;
}
