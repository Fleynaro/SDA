#include "Core/DataType/DataTypeParser.h"
#include "Core/IRcode/IRcodeDataTypeProvider.h"
#include "Core/IRcode/IRcodePrinter.h"
#include "Core/Test/Utils/TestAssertion.h"
#include "Decompiler/IRcode/Generator/IRcodeBlockGenerator.h"
#include "Decompiler/Semantics/SemanticsManager.h"
#include "Decompiler/Semantics/SemanticsProvider.h"
#include "Decompiler/Semantics/SemanticsInitializer.h"
#include "Decompiler/Test/PcodeFixture.h"

using namespace sda;
using namespace sda::test;
using namespace sda::decompiler;
using namespace ::testing;

class SemanticsTest : public PcodeFixture
{
protected:
    
};

TEST_F(SemanticsTest, Sample1) {
    auto sourcePCode = "\
        rcx:8 = COPY rcx:8 \
        rbx:8 = INT_MULT rdx:8, 4:8 \
        rbx:8 = INT_ADD rcx:8, rbx:8 \
        rbx:8 = INT_ADD rbx:8, 0x10:8 \
        STORE rbx:8, 1.0:8 \
    ";

    parseDataType("\
        EntityType = enum { \
            VEHICLE, \
            PED \
        } \
    ");
    parseDataType("\
        ['entity can be vehicle or player'] \
        Entity = struct { \
            EntityType type, \
            float[3] pos = 0x10 \
        } \
    ");
    auto globalSymbolTable = parseSymbolTable("\
        GlobalSymbolTable = {} \
    ");
    auto stackSymbolTable = parseSymbolTable("\
        StackSymbolTable = {} \
    ");
    auto instrSymbolTable = parseSymbolTable("\
        InstrSymbolTable = {} \
    ");
    auto functionSignature = dynamic_cast<SignatureDataType*>(parseDataType("\
        SetEntityVelAxisSig = signature fastcall void(Entity* entity, uint64_t idx) \
    "));
    auto functionSymbol = new FunctionSymbol(
        context, nullptr, "mainFunction", functionSignature, stackSymbolTable, instrSymbolTable);

    SemanticsManager semManager(context);
    new BaseSemanticsPropagator(&semManager);
    BaseSemanticsInitializer semInit(&semManager);
    semInit.addSymbolTable(globalSymbolTable);
    semInit.addSymbol(functionSymbol);
    auto semCtx = std::make_shared<SemanticsContext>();
    semCtx->globalSymbolTable = globalSymbolTable;
    semCtx->functionSymbol = functionSymbol;

    pcode::Block pcodeBlock;
    ircode::Block ircodeBlock(&pcodeBlock);
    TotalMemorySpace memorySpace;
    IRcodeSemanticsDataTypeProvider dataTypeProvider(&semManager);
    IRcodeBlockGenerator ircodeGen(&ircodeBlock, &memorySpace, &dataTypeProvider);

    pcode::Printer pcodePrinter(context->getPlatform()->getRegisterRepository());
    ircode::Printer ircodePrinter(&pcodePrinter);
    ircodePrinter.setOutput(std::cout);

    auto instructions = parsePcode(sourcePCode);
    for (const auto& instr : instructions) {
        ircodeGen.executePcode(&instr);

        for (auto ircodeOp : ircodeGen.getGeneratedOperations()) {
            ircodePrinter.printOperation(ircodeOp);
            ircodePrinter.newLine();

            SemanticsContextOperations ctxOps;
            ctxOps.insert({ semCtx, ircodeOp });
            semManager.propagate(ctxOps);
        }
    }

    ircodePrinter.setExtendInfo(true);
    ircodePrinter.setDataTypeProvider(&dataTypeProvider);
    ircodePrinter.newLine();
    for (auto& ircodeOp : ircodeBlock.getOperations()) {
        ircodePrinter.printOperation(ircodeOp.get());
        ircodePrinter.newLine();
    }
    semManager.print(std::cout);
}