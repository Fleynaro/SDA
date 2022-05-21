#include <iostream>
#include "Plugin/PluginLoader.h"
#include "Program.h"
#include "Project.h"
#include "Factory.h"
#include "Core/Context.h"
#include "Core/Image/AddressSpace.h"
#include "Database/Database.h"
#include "Database/Schema.h"
#include "Database/Loader.h"
#include "Database/Transaction.h"
#include "Callbacks/ContextCallbacks.h"
#include "Change.h"
#include <Zycore/Format.h>
#include <Zycore/LibC.h>
#include <Zydis/Zydis.h>

using namespace sda;

int main(int argc, char *argv[])
{
    // ZyanU8 data[] =
    // {
    //     0x51, 0x8D, 0x45, 0xFF, 0x50, 0xFF, 0x75, 0x0C, 0xFF, 0x75,
    //     0x08, 0xFF, 0x15, 0xA0, 0xA5, 0x48, 0x76, 0x85, 0xC0, 0x0F,
    //     0x88, 0xFC, 0xDA, 0x02, 0x00
    // };

    // ZydisDecoder decoder;
    // ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

    Program program;
    for (auto plugin : GetPlugins("plugins")) {
        plugin->onPluginLoaded(&program);
        program.addPlugin(std::unique_ptr<IPlugin>(plugin.get()));
    }

    auto ctx = new Context();
    auto project = new Project(&program, "Test", ctx);
    auto ctxCallbacks = new ContextCallbacks(ctx, project);
    project->getDatabase()->init();

    project->getChangeChain()->newChangeList();
    auto space1 = new AddressSpace(ctx, nullptr, "space1");
    space1->setName("space1");
    auto space2 = new AddressSpace(ctx, nullptr, "space2");
    project->getTransaction()->commit();

    project->getChangeChain()->undo();

    auto ctx2 = new Context();
    auto oldCallbacks = ctx2->setCallbacks(std::make_unique<Context::Callbacks>());
    Factory factory(ctx2);
    Loader loader(project->getDatabase(), &factory);
    loader.load();
    ctx2->setCallbacks(std::move(oldCallbacks));

    return 0;
}