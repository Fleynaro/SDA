#include <iostream>
#include "Plugin/PluginLoader.h"
#include "Program.h"
#include "Project.h"
#include "Factory.h"
#include "Core/Context.h"
#include "Core/Image/AddressSpace.h"
#include "Core/Image/Image.h"
#include "Database/Database.h"
#include "Database/Schema.h"
#include "Database/Loader.h"
#include "Database/Transaction.h"
#include "Callbacks/ContextCallbacks.h"
#include "Change.h"
#include "Disasm/Zydis/ZydisDecoderPcodeX86.h"
#include "Disasm/Zydis/ZydisInstructionRenderX86.h"
#include "Disasm/PcodeGraphBuilder.h"
#include "Disasm/VtableLookup.h"

using namespace sda;

void testPcodeDecoder() {
    auto ctx = new Context();
    auto image = new Image(
        ctx,
        std::make_unique<VectorImageRW>(
            std::vector<uint8_t>({
				0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x48, 0x83, 0xF8, 0x02, 0x0F, 0x10, 0x44, 0x24, 0x20, 0x75,
				0x05, 0x0F, 0x10, 0x44, 0x24, 0x10, 0x0F, 0x11, 0x44, 0x24, 0x10
			})
        ),
        std::make_shared<TestAnalyser>(),
        nullptr,
        "xmm registers in incomplete blocks"
    );


    // P-code decoder
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
    disasm::ZydisDecoderPcodeX86 pcodeDecoder(&decoder);
    disasm::ZydisDecoderRenderX86 decoderRender(&decoder);
    
    auto offset = image->getEntryPointOffset();

    disasm::ZydisRegisterVarnodeRender regRender;
    pcode::Instruction::StreamRender pcodeInstrRender(std::cout, &regRender);
    disasm::Instruction::StreamRender srcInstrRender(std::cout);
    while (offset < image->getSize()) {
        std::vector<uint8_t> data(0x100);
        image->getRW()->readBytesAtOffset(offset, data);
        pcodeDecoder.decode(offset, data);
        
        decoderRender.decode(data);
        srcInstrRender.render(decoderRender.getDecodedInstruction());
        std::cout << std::endl;
        auto decodedInstructions = pcodeDecoder.getDecodedInstructions();
        for (auto& decodedInstruction : decodedInstructions) {
            pcodeInstrRender.render(&decodedInstruction);
            std::cout << std::endl;
        }

        offset += pcodeDecoder.getInstructionLength();
    }



    // P-code graph builder
    disasm::PcodeGraphBuilder graphBuilder(image->getPcodeGraph(), image, &pcodeDecoder);

    class GraphCallbacks : public pcode::Graph::Callbacks {
    public:
        void onFunctionGraphCreated(pcode::FunctionGraph* functionGraph) override {
            // create function symbol here
            std::cout << "Function graph created: " << functionGraph->getEntryBlock()->getMinOffset() << std::endl;
        }

        void onFunctionGraphRemoved(pcode::FunctionGraph* functionGraph) override {
            std::cout << "Function graph removed: " << functionGraph->getEntryBlock()->getMinOffset() << std::endl;
        }
    };
    auto graphCallbacks = std::make_unique<GraphCallbacks>();
    auto oldCallbacks = image->getPcodeGraph()->setCallbacks(std::move(graphCallbacks));

    // remove old instructions
    // image->getPcodeGraph()->removeInstruction();

    // reanalyse
    auto funcCallLookupCallbacks = std::make_unique<disasm::VtableLookupCallbacks>(
        image, graphBuilder.getBlockBuilder());
    graphBuilder.getBlockBuilder()->setCallbacks(std::move(funcCallLookupCallbacks));
    graphBuilder.start({ pcode::InstructionOffset(image->getEntryPointOffset(), 0) });

    image->getPcodeGraph()->setCallbacks(std::move(oldCallbacks));
}

void testGeneral() {
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
}

int main(int argc, char *argv[])
{
    testPcodeDecoder();
    //testGeneral();
    return 0;
}