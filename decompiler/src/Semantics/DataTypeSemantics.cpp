#include "Decompiler/Semantics/DataTypeSemantics.h"
#include "Decompiler/Semantics/SemanticsManager.h"
#include "Core/DataType/PointerDataType.h"
#include "Core/DataType/StructureDataType.h"

using namespace sda;
using namespace sda::decompiler;

DataTypeSemantics::DataTypeSemantics(DataType* dataType, SymbolTable* symbolTable)
    : m_dataType(dataType)
    , m_symbolTable(symbolTable)
{
    if (!m_symbolTable) {
        if (auto pointerDt = dynamic_cast<PointerDataType*>(dataType))
            if (auto structDt = dynamic_cast<StructureDataType*>(pointerDt->getPointedType()))
                m_symbolTable = structDt->getSymbolTable();
    }
}

std::string DataTypeSemantics::getName() const {
    return "DataTypeSemantics";
}

void DataTypeSemantics::Propagator::init(
    const Context* ctx,
    const ircode::Operation* op)
{
    auto output = op->getOutput();

    if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(op)) {
        auto input = unaryOp->getInput();
        if (unaryOp->getId() == ircode::OperationId::LOAD) {
            if (auto regValue = std::dynamic_pointer_cast<ircode::Register>(input)) {
                const auto& reg = regValue->getRegister();
                auto it = ctx->storages.find({
                    CallingConvention::Storage::Read,
                    reg.getRegId()
                });
                if (it != ctx->storages.end()) {
                    // if it is a parameter
                    const auto& storageInfo = it->second;
                    auto paramIdx = storageInfo.paramIdx;
                    auto paramSymbol = ctx->signatureDt->getParameters()[paramIdx];
                    auto paramSymbolHolder = getManager()->getHolder(paramSymbol);
                    
                    auto sem = paramSymbolHolder->find("DataTypeSemantics");
                    if (!sem) {
                        sem = getManager()->addSemantics(
                            std::make_unique<DataTypeSemantics>(paramSymbol->getDataType()));
                        paramSymbolHolder->add(sem, true);
                    }

                    getManager()->getHolder(output)->add(sem);
                    getManager()->addSymbol(paramSymbol, output);
                } else {
                    // if it is a stack or global variable
                    SymbolTable* symbolTable = ctx->stackSymbolTable;
                    if (reg.getRegId() == Register::InstructionPointerId)
                        symbolTable = ctx->globalSymbolTable;
                    if (symbolTable) {
                        auto voidDt = findDataType("void");
                        auto sem = getManager()->addSemantics(
                            std::make_unique<DataTypeSemantics>(voidDt->getPointerTo(), symbolTable));
                        getManager()->getHolder(output)->add(sem, true);
                    }
                }
            }
        }
    }
}

void DataTypeSemantics::Propagator::propagate(
    const Context* ctx,
    const ircode::Operation* op,
    const Semantics* sem,
    std::list<std::shared_ptr<ircode::Variable>>& affectedVars)
{
    auto output = op->getOutput();

}