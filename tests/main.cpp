#include <Windows.h>
#include "gtest/gtest.h"

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
    
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    //::testing::GTEST_FLAG(filter) = "Test_Dec_*";
	return RUN_ALL_TESTS();
}