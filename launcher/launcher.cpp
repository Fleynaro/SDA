#include <iostream>
#include "Program.h"
#include "Project.h"
#include "Core/Context.h"
#include "Core/Function.h"

using namespace sda;

int main(int argc, char *argv[])
{
    Program program;
    auto project = new Project(&program, "Test");

    auto ctx = new Context();
    auto func1 = new Function(ctx, 1000);
    auto func2 = new Function(ctx, 1002);

    for(auto f : *ctx->getFunctions()) {
        std::cout << f->getOffset() << std::endl;
    }

    boost::json::object root;
    root["total"] = 5;
    
    auto functions = boost::json::array();
    for(auto f : *ctx->getFunctions()) {
        if(auto serFunc = dynamic_cast<ISerializable*>(f)) {
            boost::json::object data;
            serFunc->serialize(data);
            functions.push_back(data);
        }
    }
    root["functions"] = functions;


    // print json
    std::cout << root << std::endl;

    return 0;
}