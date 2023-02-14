#pragma once
#include "SDA/Core/Pcode/PcodeInstruction.h"
#include "SDA/Core/Pcode/PcodeBlock.h"
#include "SDA/Core/Pcode/PcodeFunctionGraph.h"
#include "SDA/Core/Pcode/PcodeGraph.h"
#include "SDA/Core/Pcode/PcodeParser.h"
#include "SDA/Core/Pcode/PcodePrinter.h"

namespace sda::bind
{
    class PcodeVarnodeBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<pcode::Varnode, v8pp::shared_ptr_traits>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("size", &pcode::Varnode::getSize)
                .property("isRegister", &pcode::Varnode::isRegister);
            ObjectLookupTableShared::Register(cl);
            RegisterClassName(cl, "DataType");
            module.class_("PcodeVarnode", cl);
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
                .property("level", &pcode::Block::getLevel)
                .property("minOffset", &pcode::Block::getMinOffset)
                .property("maxOffset", &pcode::Block::getMaxOffset, &pcode::Block::setMaxOffset)
                .property("nearNextBlock", &pcode::Block::getNearNextBlock, &pcode::Block::setNearNextBlock)
                .property("farNextBlock", &pcode::Block::getFarNextBlock, &pcode::Block::setFarNextBlock)
                .property("referencedBlocks", &pcode::Block::getReferencedBlocks)
                .property("instructions", &pcode::Block::getInstructions)
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
                .property("entryBlock", &pcode::FunctionGraph::getEntryBlock)
                .property("referencedGraphsTo", &pcode::FunctionGraph::getReferencesTo)
                .property("referencedGraphsFrom", &pcode::FunctionGraph::getReferencesFrom)
                .method("addReferencedGraphFrom", &pcode::FunctionGraph::addReferenceFrom)
                .method("removeReferencedGraphFrom", &pcode::FunctionGraph::removeReferencedGraphFrom);
            module.class_("PcodeFunctionGraph", cl);
        }
    };

    class PcodeGraphCallbacksBind
    {
        using Callbacks = pcode::Graph::Callbacks;
        class CallbacksJsImpl : public Callbacks
        {
        public:
            std::shared_ptr<Callbacks> m_prevCallbacks = std::make_shared<Callbacks>();
            Callback m_onFunctionGraphCreated;
            Callback m_onFunctionGraphRemoved;

            void onFunctionGraphCreated(pcode::FunctionGraph* graph) override {
                m_prevCallbacks->onFunctionGraphCreated(graph);
                if (m_onFunctionGraphCreated.isDefined()) {
                    m_onFunctionGraphCreated.call(graph);
                }
            }

            void onFunctionGraphRemoved(pcode::FunctionGraph* graph) override {
                m_prevCallbacks->onFunctionGraphRemoved(graph);
                if (m_onFunctionGraphRemoved.isDefined()) {
                    m_onFunctionGraphRemoved.call(graph);
                }
            }
        };
        
        static auto New() {
            return std::make_shared<CallbacksJsImpl>();
        }
    public:
        static void Init(v8pp::module& module) {
            {
                auto cl = NewClass<Callbacks, v8pp::shared_ptr_traits>(module);
                cl
                    .auto_wrap_object_ptrs(true)
                    .method("onFunctionGraphCreated", &Callbacks::onFunctionGraphCreated)
                    .method("onFunctionGraphRemoved", &Callbacks::onFunctionGraphRemoved);
                module.class_("PcodeGraphCallbacks", cl);
            }
            {
                auto cl = NewClass<CallbacksJsImpl, v8pp::shared_ptr_traits>(module);
                cl
                    .inherit<Callbacks>()
                    .var("prevCallbacks", &CallbacksJsImpl::m_prevCallbacks)
                    .static_method("New", &New);
                Callback::Register(cl, "onFunctionGraphCreated", &CallbacksJsImpl::m_onFunctionGraphCreated);
                Callback::Register(cl, "onFunctionGraphRemoved", &CallbacksJsImpl::m_onFunctionGraphRemoved);
                module.class_("PcodeGraphCallbacksImpl", cl);
            }
        }
    };

    class PcodeGraphBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<pcode::Graph>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("callbacks", &pcode::Graph::getCallbacks, &pcode::Graph::setCallbacks)
                .method("addInstruction", &pcode::Graph::addInstruction)
                .method("removeInstruction", &pcode::Graph::removeInstruction)
                .method("getInstructionAt", &pcode::Graph::getInstructionAt)
                .method("getInstructionsAt", &pcode::Graph::getInstructionsAt)
                .method("createBlock", &pcode::Graph::createBlock)
                .method("removeBlock", &pcode::Graph::removeBlock)
                .method("getBlockAt", &pcode::Graph::getBlockAt)
                .method("createFunctionGraph", &pcode::Graph::createFunctionGraph)
                .method("removeFunctionGraph", &pcode::Graph::removeFunctionGraph);
            module.class_("PcodeGraph", cl);
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
        };

        static auto New(std::shared_ptr<RegisterRepository> regRepo) {
            auto printer = new PrinterJs(regRepo.get());
            PrinterJs::ObjectInit(printer);
            return ExportObject(printer);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<PrinterJs>(module);
            PrinterJs::ClassInit(cl);
            cl
                .method("printInstruction", std::function([](PrinterJs* printer, const pcode::Instruction* instruction) {
                    printer->pcode::Printer::printInstruction(instruction);
                }))
                .method("printVarnode", std::function([](PrinterJs* printer, std::shared_ptr<pcode::Varnode> varnode, bool printSizeAndOffset) {
                    printer->pcode::Printer::printVarnode(varnode, printSizeAndOffset);
                }))
                .static_method("Print", &PrinterJs::Print)
                .static_method("New", &New);
            Callback::Register(cl, "printInstructionImpl", &PrinterJs::m_printInstructionImpl);
            Callback::Register(cl, "printVarnodeImpl", &PrinterJs::m_printVarnodeImpl);
            module.class_("PcodePrinter", cl);
        }
    };
};