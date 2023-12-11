#pragma once
#include "SDA/Core/Pcode/PcodeInstruction.h"
#include "SDA/Core/Pcode/PcodeBlock.h"
#include "SDA/Core/Pcode/PcodeFunctionGraph.h"
#include "SDA/Core/Pcode/PcodeGraph.h"
#include "SDA/Core/Pcode/PcodeStructurer.h"
#include "SDA/Core/Pcode/PcodeParser.h"
#include "SDA/Core/Pcode/PcodePrinter.h"
#include "SDA/Core/IRcode/IRcodePrinter.h"

namespace sda::bind
{
    class PcodeVarnodeBind
    {
        static void InitVarnode(v8pp::module& module) {
            auto cl = NewClass<pcode::Varnode, v8pp::shared_ptr_traits>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("size", &pcode::Varnode::getSize)
                .property("isRegister", &pcode::Varnode::isRegister);
            module.class_("PcodeVarnode", cl);
        }

        static void InitConstantVarnode(v8pp::module& module) {
            auto cl = NewClass<pcode::ConstantVarnode, v8pp::shared_ptr_traits>(module);
            cl
                .inherit<pcode::Varnode>()
                .auto_wrap_object_ptrs(true)
                .property("value", &pcode::ConstantVarnode::getValue);
            module.class_("PcodeConstantVarnode", cl);
        }

        static void InitRegisterVarnode(v8pp::module& module) {
            auto cl = NewClass<pcode::RegisterVarnode, v8pp::shared_ptr_traits>(module);
            cl
                .inherit<pcode::Varnode>()
                .auto_wrap_object_ptrs(true);
            module.class_("PcodeRegisterVarnode", cl);
        }
    public:
        static void Init(v8pp::module& module) {
            InitVarnode(module);
            InitConstantVarnode(module);
            InitRegisterVarnode(module);
        }
    };

    class PcodeInstructionBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<pcode::Instruction>(module);
            cl
                .auto_wrap_objects(true)
                .auto_wrap_object_ptrs(true)
                .property("id", &pcode::Instruction::getId)
                .property("input0", &pcode::Instruction::getInput0)
                .property("input1", &pcode::Instruction::getInput1)
                .property("output", &pcode::Instruction::getOutput)
                .property("offset", &pcode::Instruction::getOffset);
            module.class_("PcodeInstruction", cl);
        }
    };

    class PcodeBlockBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<pcode::Block>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("name", &pcode::Block::getName)
                .property("index", &pcode::Block::getIndex)
                .property("graph", &pcode::Block::getGraph)
                .property("functionGraph", &pcode::Block::getFunctionGraph)
                .property("minOffset", &pcode::Block::getMinOffset)
                .property("maxOffset", &pcode::Block::getMaxOffset, &pcode::Block::setMaxOffset)
                .property("nearNextBlock", &pcode::Block::getNearNextBlock, &pcode::Block::setNearNextBlock)
                .property("farNextBlock", &pcode::Block::getFarNextBlock, &pcode::Block::setFarNextBlock)
                .property("referencedBlocks", &pcode::Block::getReferencedBlocks)
                .property("instructions", &pcode::Block::getInstructions)
                .property("lastInstruction", &pcode::Block::getLastInstruction)
                .method("contains", &pcode::Block::contains);
            module.class_("PcodeBlock", cl);
        }
    };

    class PcodeFunctionGraphBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<pcode::FunctionGraph>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("name", &pcode::FunctionGraph::getName)
                .property("entryBlock", &pcode::FunctionGraph::getEntryBlock)
                .property("entryOffset", &pcode::FunctionGraph::getEntryOffset)
                .property("graph", &pcode::FunctionGraph::getGraph)
                .property("referencedGraphsTo", &pcode::FunctionGraph::getReferencesTo)
                .property("referencedGraphsFrom", &pcode::FunctionGraph::getReferencesFrom);
            module.class_("PcodeFunctionGraph", cl);
        }
    };

    class PcodeGraphBind
    {
        static void ExploreImage(pcode::Graph* graph, pcode::InstructionOffset startOffset, Image* image) {
            pcode::ImageInstructionProvider provider(image);
            graph->explore(startOffset, &provider);
        }

        static void ExploreInstructions(pcode::Graph* graph, pcode::InstructionOffset startOffset, const std::list<pcode::Instruction>& instructions) {
            pcode::ListInstructionProvider provider(instructions);
            graph->explore(startOffset, &provider);
        }

        static auto New(std::shared_ptr<EventPipe> eventPipe, Platform* platform) {
            auto graph = new pcode::Graph(eventPipe, platform);
            ObjectLookupTableRaw::AddObject(graph);
            // TODO: ObjectLookupTableRaw::RemoveObject(graph); (handle event PcodeGraphRemoved)
            return ExportObject(graph);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<pcode::Graph>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("platform", &pcode::Graph::getPlatform)
                .method("exploreImage", &ExploreImage)
                .method("exploreInstructions", &ExploreInstructions)
                .method("addInstruction", &pcode::Graph::addInstruction)
                .method("removeInstruction", &pcode::Graph::removeInstruction)
                .method("getInstructionAt", &pcode::Graph::getInstructionAt)
                .method("getInstructionsAt", &pcode::Graph::getInstructionsAt)
                .method("createBlock", &pcode::Graph::createBlock)
                .method("removeBlock", &pcode::Graph::removeBlock)
                .method("getBlockAt", &pcode::Graph::getBlockAt)
                .method("createFunctionGraph", &pcode::Graph::createFunctionGraph)
                .method("removeFunctionGraph", &pcode::Graph::removeFunctionGraph)
                .method("getFunctionGraphAt", &pcode::Graph::getFunctionGraphAt)
                .static_method("New", &New);
            ObjectLookupTableRaw::Register(cl);
            RegisterClassName(cl, "PcodeGraph");
            module.class_("PcodeGraph", cl);
        }
    };

    class PcodeStructurer
    {
        static void InitStructBlock(v8pp::module& module) {
            auto cl = NewClass<pcode::StructBlock>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("name", &pcode::StructBlock::getName)
                .property("pcodeBlock", &pcode::StructBlock::getPcodeBlock)
                .property("nearNextBlock", &pcode::StructBlock::getNearNextBlock)
                .property("farNextBlock", &pcode::StructBlock::getFarNextBlock)
                .property("referencedBlocks", &pcode::StructBlock::getReferencedBlocks)
                .property("goto", &pcode::StructBlock::getGoto)
                .property("gotoType", &pcode::StructBlock::getGotoType)
                .property("gotoRefBlocks", &pcode::StructBlock::getGotoReferencedBlocks)
                .property("parent", &pcode::StructBlock::getParent)
                .property("top", &pcode::StructBlock::getTop)
                .property("bottom", &pcode::StructBlock::getBottom)
                .property("leafs", [](pcode::StructBlock& self) {
                    std::list<pcode::StructBlock*> leafs;
                    self.getLeafs(leafs);
                    return leafs;
                });
            module.class_("PcodeStructBlock", cl);
        }

        static void InitStructBlockSequence(v8pp::module& module) {
            auto cl = NewClass<pcode::StructBlockSequence>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .inherit<pcode::StructBlock>()
                .property("blocks", &pcode::StructBlockSequence::getBlocks);
            module.class_("PcodeStructBlockSequence", cl);
        }

        static void InitStructBlockIf(v8pp::module& module) {
            auto cl = NewClass<pcode::StructBlockIf>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .inherit<pcode::StructBlock>()
                .property("conditionBlock", &pcode::StructBlockIf::getCondBlock)
                .property("thenBlock", &pcode::StructBlockIf::getThenBlock)
                .property("elseBlock", &pcode::StructBlockIf::getElseBlock)
                .property("inverted", &pcode::StructBlockIf::isInverted);
            module.class_("PcodeStructBlockIf", cl);
        }

        static void InitStructBlockWhile(v8pp::module& module) {
            auto cl = NewClass<pcode::StructBlockWhile>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .inherit<pcode::StructBlock>()
                .property("bodyBlock", &pcode::StructBlockWhile::getBodyBlock);
            module.class_("PcodeStructBlockWhile", cl);
        }

        static auto New() {
            return ExportObject(new pcode::StructTree());
        }

        static void InitStructTree(v8pp::module& module) {
            auto cl = NewClass<pcode::StructTree>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("entryBlock", &pcode::StructTree::getEntryBlock)
                .method("init", std::function([](pcode::StructTree* structTree, pcode::FunctionGraph* funcGraph) {
                    pcode::Structurer structurer(funcGraph, structTree);
                    structurer.start();
                }))
                .static_method("New", &New);
            module.class_("PcodeStructTree", cl);
        }
    public:
        static void Init(v8pp::module& module) {
            InitStructBlock(module);
            InitStructBlockSequence(module);
            InitStructBlockIf(module);
            InitStructBlockWhile(module);
            InitStructTree(module);
        }
    };

    class PcodeStructTreePrinterBind : public AbstractPrinterBind
    {
        class PrinterJs : public AbstractPrinterJs<pcode::StructTreePrinter> {
        public:
            Callback m_printStructBlockImpl;

            using AbstractPrinterJs::AbstractPrinterJs;

            void printStructBlock(pcode::StructBlock* block) override {
                if (m_printStructBlockImpl.isDefined()) {
                    m_printStructBlockImpl.call(block);
                } else {
                    AbstractPrinterJs::printStructBlock(block);
                }
            }

            static auto New() {
                auto printer = new PrinterJs();
                ObjectInit(printer);
                return ExportObject(printer);
            }

            static void Init(v8pp::module& module) {
                auto cl = NewClass<PrinterJs>(module);
                ClassInit(cl);
                cl
                    .static_method("New", &New);
                Callback::Register(cl, "printStructBlockImpl", &PrinterJs::m_printStructBlockImpl);
                module.class_("PcodeStructTreePrinterJs", cl);
            }
        };
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<pcode::StructTreePrinter>(module);
            cl
                .inherit<utils::AbstractPrinter>()
                .method("printStructBlock", std::function([](pcode::StructTreePrinter* printer, pcode::StructBlock* block) {
                    printer->pcode::StructTreePrinter::printStructBlock(block);
                }))
                .method("printStructTree", std::function([](pcode::StructTreePrinter* printer, pcode::StructTree* tree) {
                    printer->pcode::StructTreePrinter::printStructTree(tree);
                }));
            module.class_("PcodeStructTreePrinter", cl);
            PrinterJs::Init(module);
        }
    };

    class PcodeParserBind
    {
        static auto Parse(const std::string& text, std::shared_ptr<RegisterRepository> regRepo) {
            return pcode::Parser::Parse(text, regRepo.get());
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<pcode::Parser>(module);
            cl
                .static_method("Parse", &Parse);
            module.class_("PcodeParser", cl);
        }
    };

    class PcodePrinterBind : public AbstractPrinterBind
    {
        class PrinterJs : public AbstractPrinterJs<pcode::Printer> {
        public:
            Callback m_printInstructionImpl;
            Callback m_printVarnodeImpl;

            using AbstractPrinterJs::AbstractPrinterJs;

            void printInstruction(const pcode::Instruction* instruction) const override {
                if (m_printInstructionImpl.isDefined()) {
                    m_printInstructionImpl.call(instruction);
                } else {
                    AbstractPrinterJs::printInstruction(instruction);
                }
            }

            void printVarnode(std::shared_ptr<pcode::Varnode> varnode, bool printSizeAndOffset) const override {
                if (m_printVarnodeImpl.isDefined()) {
                    m_printVarnodeImpl.call(varnode, printSizeAndOffset);
                } else {
                    AbstractPrinterJs::printVarnode(varnode, printSizeAndOffset);
                }
            }

            static auto New(std::shared_ptr<RegisterRepository> regRepo) {
                auto printer = new PrinterJs(regRepo.get());
                ObjectInit(printer);
                return ExportObject(printer);
            }

            static void Init(v8pp::module& module) {
                auto cl = NewClass<PrinterJs>(module);
                ClassInit(cl);
                cl
                    .static_method("New", &New);
                Callback::Register(cl, "printInstructionImpl", &PrinterJs::m_printInstructionImpl);
                Callback::Register(cl, "printVarnodeImpl", &PrinterJs::m_printVarnodeImpl);
                module.class_("PcodePrinterJs", cl);
            }
        };
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<pcode::Printer>(module);
            cl
                .inherit<utils::AbstractPrinter>()
                .method("combineWithStructPrinter", std::function([](pcode::Printer* printer, pcode::StructTreePrinter* structPrinter) {
                    printer->setParentPrinter(structPrinter);
                    structPrinter->setCodePrinter(printer->getCodePrinter());
                    structPrinter->setConditionPrinter(printer->getConditionPrinter());
                }))
                .method("printInstruction", std::function([](pcode::Printer* printer, const pcode::Instruction* instruction) {
                    printer->pcode::Printer::printInstruction(instruction);
                }))
                .method("printVarnode", std::function([](pcode::Printer* printer, std::shared_ptr<pcode::Varnode> varnode, bool printSizeAndOffset) {
                    printer->pcode::Printer::printVarnode(varnode, printSizeAndOffset);
                }))
                .static_method("Print", &pcode::Printer::Print);
            module.class_("PcodePrinter", cl);
            PrinterJs::Init(module);
        }
    };

    static void PcodeBindInit(v8pp::module& module) {
        PcodeVarnodeBind::Init(module);
        PcodeInstructionBind::Init(module);
        PcodeBlockBind::Init(module);
        PcodeFunctionGraphBind::Init(module);
        PcodeGraphBind::Init(module);
        PcodeStructurer::Init(module);
        PcodeStructTreePrinterBind::Init(module);
        PcodeParserBind::Init(module);
        PcodePrinterBind::Init(module);
    }
};