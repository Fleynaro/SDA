#include <iostream>
#include "Program.h"
#include "Project.h"
#include "Core/Context.h"
#include "Core/Function.h"

#include <fstream>

using namespace sda;

int main(int argc, char *argv[])
{
    Program program;
    auto project = new Project(&program, "Test");

    auto ctx = new Context();
    new Function(ctx, 1000);
    new Function(ctx, 1002);

    for(auto f : *ctx->getFunctions()) {
        std::cout << f->getOffset() << std::endl;
    }

    return 0;
}

void test1() {
    auto ctx = new Context();

    auto func = new Function(ctx, 1000);
    {
        ISerializable* obj = func;
        std::ofstream ofs("filename.dat");
        Serialize(obj, ofs);
    }

    Function* func2;
    {
        std::ifstream ifs("filename.dat");
        auto obj = Deserialize(ifs);
        func2 = dynamic_cast<Function*>(obj);
    }
}