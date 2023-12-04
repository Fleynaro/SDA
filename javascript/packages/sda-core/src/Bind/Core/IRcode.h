#pragma once
#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/IRcode/IRcodeContextSync.h"
#include "SDA/Core/IRcode/IRcodePcodeSync.h"
#include "SDA/Core/IRcode/IRcodePrinter.h"

namespace sda::bind
{
    class IRcodeValueBind
    {
        static void InitValue(v8pp::module& module) {
            auto cl = NewClass<ircode::Value, v8pp::shared_ptr_traits>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("type", &ircode::Value::getType)
                .property("size", &ircode::Value::getSize)
                .property("hash", &ircode::Value::getHash);
            module.class_("IRcodeValue", cl);
        }

        static void InitConstant(v8pp::module& module) {
            auto cl = NewClass<ircode::Constant, v8pp::shared_ptr_traits>(module);
            cl
                .inherit<ircode::Value>()
                .auto_wrap_object_ptrs(true)
                .property("constVarnode", &ircode::Constant::getConstVarnode);
            module.class_("IRcodeConstant", cl);
        }

        static void InitRegister(v8pp::module& module) {
            auto cl = NewClass<ircode::Register, v8pp::shared_ptr_traits>(module);
            cl
                .inherit<ircode::Value>()
                .auto_wrap_object_ptrs(true)
                .property("regVarnode", &ircode::Register::getRegVarnode);
            module.class_("IRcodeRegister", cl);
        }

        static void InitVariable(v8pp::module& module) {
            auto cl = NewClass<ircode::Variable, v8pp::shared_ptr_traits>(module);
            cl
                .inherit<ircode::Value>()
                .auto_wrap_object_ptrs(true)
                .property("id", &ircode::Variable::getId)
                .property("name", std::function([](ircode::Variable& self) {
                    return self.getName(false);
                }))
                .property("fullName", std::function([](ircode::Variable& self) {
                    return self.getName(true);
                }))
                .property("isUsed", &ircode::Variable::isUsed)
                .property("sourceOperation", &ircode::Variable::getSourceOperation);
            module.class_("IRcodeVariable", cl);
        }
    public:
        static void Init(v8pp::module& module) {
            InitValue(module);
            InitConstant(module);
            InitRegister(module);
            InitVariable(module);
        }
    };

    class IRcodeOperationBind
    {
        static void InitOperation(v8pp::module& module) {
            auto cl = NewClass<ircode::Operation>(module);
            cl
                .auto_wrap_objects(true)
                .auto_wrap_object_ptrs(true)
                .property("id", &ircode::Operation::getId)
                .property("hash", &ircode::Operation::getHash)
                .property("output", &ircode::Operation::getOutput)
                .property("block", &ircode::Operation::getBlock)
                .property("pcodeInstruction", &ircode::Operation::getPcodeInstruction);
            module.class_("IRcodeOperation", cl);
        }

        static void InitUnaryOperation(v8pp::module& module) {
            auto cl = NewClass<ircode::UnaryOperation>(module);
            cl
                .inherit<ircode::Operation>()
                .auto_wrap_objects(true)
                .auto_wrap_object_ptrs(true)
                .property("input", &ircode::UnaryOperation::getInput);
            module.class_("IRcodeUnaryOperation", cl);
        }

        static void InitBinaryOperation(v8pp::module& module) {
            auto cl = NewClass<ircode::BinaryOperation>(module);
            cl
                .inherit<ircode::Operation>()
                .auto_wrap_objects(true)
                .auto_wrap_object_ptrs(true)
                .property("input1", &ircode::BinaryOperation::getInput1)
                .property("input2", &ircode::BinaryOperation::getInput2);
            module.class_("IRcodeBinaryOperation", cl);
        }
    public:
        static void Init(v8pp::module& module) {
            InitOperation(module);
            InitUnaryOperation(module);
            InitBinaryOperation(module);
        }
    };

    class IRcodeBlockBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ircode::Block>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("name", &ircode::Block::getName)
                .property("hash", &ircode::Block::getHash)
                .property("pcodeBlock", &ircode::Block::getPcodeBlock)
                .property("function", &ircode::Block::getFunction)
                .property("nearNextBlock", &ircode::Block::getNearNextBlock)
                .property("farNextBlock", &ircode::Block::getFarNextBlock)
                .property("referencedBlocks", &ircode::Block::getReferencedBlocks)
                .property("operations", std::function([](ircode::Block& self) {
                    return to_v8(self.getOperations());
                }));
            module.class_("IRcodeBlock", cl);
        }
    };

    class IRcodeFunctionBind
    {
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ircode::Function>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("name", &ircode::Function::getName)
                .property("entryBlock", &ircode::Function::getEntryBlock)
                .property("entryOffset", &ircode::Function::getEntryOffset)
                .property("program", &ircode::Function::getProgram)
                .property("funcGraph", &ircode::Function::getFunctionGraph)
                .property("funcSymbol", &ircode::Function::getFunctionSymbol)
                .property("variables", &ircode::Function::getVariables)
                .property("paramVariables", &ircode::Function::getParamVariables)
                .property("returnVariable", &ircode::Function::getReturnVariable)
                .method("findVariableById", &ircode::Function::findVariableById);
            module.class_("IRcodeFunction", cl);
        }
    };

    class IRcodeProgramBind
    {
        static auto New(pcode::Graph* graph, SymbolTable* globalSymbolTable) {
            auto program = new ircode::Program(graph, globalSymbolTable);
            ObjectLookupTableRaw::AddObject(program);
            // TODO: ObjectLookupTableRaw::RemoveObject(program); (handle event IRcodeProgramRemoved)
            return ExportObject(program);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ircode::Program>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("eventPipe", &ircode::Program::getEventPipe)
                .property("graph", &ircode::Program::getGraph)
                .property("globalSymbolTable", &ircode::Program::getGlobalSymbolTable)
                .method("getFunctionAt", &ircode::Program::getFunctionAt)
                .method("getFunctionsByCallInstruction", &ircode::Program::getFunctionsByCallInstruction)
                .method("getCallsRefToFunction", &ircode::Program::getCallsRefToFunction)
                .static_method("New", &New);
            ObjectLookupTableRaw::Register(cl);
            RegisterClassName(cl, "IRcodeProgram");
            module.class_("IRcodeProgram", cl);
        }
    };

    class IRcodeContextSyncBind
    {
        static auto New(ircode::Program* program, SymbolTable* globalSymbolTable, std::shared_ptr<CallingConvention> callingConvention) {
            auto ctxSync = new ircode::ContextSync(program, globalSymbolTable, callingConvention);
            return ExportObject(ctxSync);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ircode::ContextSync>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("eventPipe", &ircode::ContextSync::getEventPipe)
                .static_method("New", &New);
            module.class_("IRcodeContextSync", cl);
        }
    };

    class IRcodePcodeSyncBind
    {
        static auto New(ircode::Program* program) {
            auto pcodeSync = new ircode::PcodeSync(program);
            return ExportObject(pcodeSync);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ircode::PcodeSync>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("eventPipe", &ircode::PcodeSync::getEventPipe)
                .static_method("New", &New);
            module.class_("IRcodePcodeSync", cl);
        }
    };

    class IRcodePrinterBind : public AbstractPrinterBind
    {
        class PrinterJs : public AbstractPrinterJs<ircode::Printer> {
        public:
            Callback m_printBlockImpl;
            Callback m_printOperationImpl;
            Callback m_printValueImpl;
            Callback m_printLinearExprImpl;

            using AbstractPrinterJs::AbstractPrinterJs;

            void printBlock(ircode::Block* block, size_t level) override {
                if (m_printBlockImpl.isDefined()) {
                    m_printBlockImpl.call(block, level);
                } else {
                    AbstractPrinterJs::printBlock(block, level);
                }
            }

            void printOperation(const ircode::Operation* operation) override {
                if (m_printOperationImpl.isDefined()) {
                    m_printOperationImpl.call(operation);
                } else {
                    AbstractPrinterJs::printOperation(operation);
                }
            }

            void printValue(std::shared_ptr<ircode::Value> value, bool extended) override {
                if (m_printValueImpl.isDefined()) {
                    m_printValueImpl.call(value, extended);
                } else {
                    AbstractPrinterJs::printValue(value, extended);
                }
            }

            void printLinearExpr(const ircode::LinearExpression* linearExpr) override {
                if (m_printLinearExprImpl.isDefined()) {
                    m_printLinearExprImpl.call(linearExpr);
                } else {
                    AbstractPrinterJs::printLinearExpr(linearExpr);
                }
            }

            static auto New(pcode::Printer* pcodePrinter) {
                auto printer = new PrinterJs(pcodePrinter);
                ObjectInit(printer);
                return ExportObject(printer);
            }

            static void Init(v8pp::module& module) {
                auto cl = NewClass<PrinterJs>(module);
                ClassInit(cl);
                cl
                    .static_method("New", &New);
                Callback::Register(cl, "printBlockImpl", &PrinterJs::m_printBlockImpl);
                Callback::Register(cl, "printOperationImpl", &PrinterJs::m_printOperationImpl);
                Callback::Register(cl, "printValueImpl", &PrinterJs::m_printValueImpl);
                Callback::Register(cl, "printLinearExprImpl", &PrinterJs::m_printLinearExprImpl);
                module.class_("IRcodePrinterJs", cl);
            }
        };
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ircode::Printer>(module);
            cl
                .inherit<utils::AbstractPrinter>()
                .method("combineWithStructPrinter", std::function([](ircode::Printer* printer, pcode::StructTreePrinter* structPrinter, ircode::Function* function) {
                    printer->setParentPrinter(structPrinter);
                    structPrinter->setCodePrinter(printer->getCodePrinter(function));
                    structPrinter->setConditionPrinter(printer->getConditionPrinter(function));
                }))
                .method("printOperation", std::function([](ircode::Printer* printer, const ircode::Operation* operation) {
                    printer->ircode::Printer::printOperation(operation);
                }))
                .method("printValue", std::function([](ircode::Printer* printer, std::shared_ptr<ircode::Value> value, bool extended) {
                    printer->ircode::Printer::printValue(value, extended);
                }));
            module.class_("IRcodePrinter", cl);
            PrinterJs::Init(module);
        }
    };

    static void PrintIRcodeInit(v8pp::module& module) {
        module.function("PrintIRcode", &sda::ircode::PrintIRcode);
    }

    static void IRcodeBindInit(v8pp::module& module) {
        IRcodeValueBind::Init(module);
        IRcodeOperationBind::Init(module);
        IRcodeBlockBind::Init(module);
        IRcodeFunctionBind::Init(module);
        IRcodeProgramBind::Init(module);
        IRcodeContextSyncBind::Init(module);
        IRcodePcodeSyncBind::Init(module);
        IRcodePrinterBind::Init(module);
        PrintIRcodeInit(module);
    }
};