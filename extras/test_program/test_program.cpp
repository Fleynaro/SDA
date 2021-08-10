#include <iostream>
#include <Windows.h>

using namespace std;

int main()
{
    printf("Hello! This is a test program.\n\n");
	
    int idx = 1;
    while(true) {
        printf("out: %i (rand = %i, some value = %i)\n", idx, rand(), idx * idx * 3 - 1);
        idx++;
        Sleep(1000);
    }
	
    return 0;
}