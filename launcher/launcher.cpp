#include <iostream>
#include "Program.h"
#include "Project.h"
#include "Core/Context.h"
#include "Core/Function.h"
#include "Database/Database.h"
#include "Database/Schema.h"
#include "Database/Loader.h"
#include "Database/Transaction.h"
#include "Callbacks/ContextCallbacks.h"

using namespace sda;

int main(int argc, char *argv[])
{
    Program program;
    auto ctx = new Context();
    auto project = new Project(&program, "Test", ctx);
    auto ctxCallbacks = new ContextCallbacks(ctx, project);
    project->getDatabase()->init();

    auto func1 = new Function(ctx, 1000);
    auto func2 = new Function(ctx, 1002);
    project->getTransaction()->commit();

    auto ctx2 = new Context();
    Loader loader(project->getDatabase(), ctx2);
    loader.load();

    return 0;
}