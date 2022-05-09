#include <iostream>
#include "Program.h"
#include "Project.h"
#include "Core/Context.h"
#include "Core/Function.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>

int main(int argc, char *argv[])
{
    using namespace sda;
    Program program;
    auto project = Project::Create(&program, "Test");

    auto ctx = new Context();
    
    auto func = Function::Create(ctx, 1000);
    {
        std::ofstream ofs("filename.dat");
        boost::archive::text_oarchive oa(ofs);
        oa << func;
    }

    auto func2 = Function::Create(ctx, 0);
    {
        std::ifstream ifs("filename.dat");
        boost::archive::text_iarchive ia(ifs);
        ia >> func2;
    }

    return 0;
}