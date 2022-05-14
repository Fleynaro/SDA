#include <iostream>
#include "Program.h"
#include "Project.h"
#include "Factory.h"
#include "Core/Context.h"
#include "Core/Function.h"
#include "Database/Database.h"
#include "Database/Schema.h"
#include "Database/Loader.h"
#include "Database/Transaction.h"
#include "Callbacks/ContextCallbacks.h"
#include "Change.h"

using namespace sda;

int main(int argc, char *argv[])
{
    Program program;
    auto ctx = new Context();
    auto project = new Project(&program, "Test", ctx);
    auto ctxCallbacks = new ContextCallbacks(ctx, project);
    project->getDatabase()->init();

    project->getChangeChain()->newChangeList();
    auto func1 = new Function(ctx, nullptr, 1000);
    func1->setOffset(1001);
    auto func2 = new Function(ctx, nullptr, 1002);
    project->getTransaction()->commit();

    project->getChangeChain()->undo();

    auto ctx2 = new Context();
    Loader loader(project->getDatabase(), ctx2, program.getFactory());
    loader.load();

    return 0;
}