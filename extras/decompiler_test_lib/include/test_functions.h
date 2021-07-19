#include <cstdint>

class TestA {
public:
	int a = 10;
	int b = 20;
	long long c = 1000;
	uint64_t arr[3];

	TestA() = default;
};

class TestB {
public:
	int b = 0;
	TestA* a = new TestA;
	TestA arr2[2][2];

	TestB() = default;
};

class TestC {
public:
	int c = 0;
	TestB* b = new TestB;

	TestC() = default;
};

int Test_SimpleFunc(int a, int b);

int Test_StructsAndArray(TestA* a);

int Test_Array();

int Test_Switch();

int Test_FloatingPoints();

int Test_Cycles();