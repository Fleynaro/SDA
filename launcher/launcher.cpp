#include <iostream>
#include "Program.h"
#include "Project.h"
#include "Core/Context.h"
#include "Core/Function.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>

using namespace sda;

BOOST_CLASS_EXPORT(Function)

int main(int argc, char *argv[])
{
    Program program;
    auto project = Project::Create(&program, "Test");

    auto ctx = new Context();

    auto func = Function::Create(ctx, 1000);
    IObject* obj = func;
    {
        std::ofstream ofs("filename.dat");
        boost::archive::text_oarchive oa(ofs);
        oa << obj;
    }

    auto func2 = Function::Create(ctx, 0);
    IObject* obj2;
    {
        std::ifstream ifs("filename.dat");
        boost::archive::text_iarchive ia(ifs);
        ia >> obj2;
    }
    func2 = dynamic_cast<Function*>(obj2);

    return 0;
}