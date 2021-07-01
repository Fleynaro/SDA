#pragma once
#include <decompiler/Decompiler.h>
#include <decompiler/LinearView/DecLinearView.h>
#include <decompiler/LinearView/DecLinearViewOptimization.h>
#include <decompiler/LinearView/DecLinearViewSimpleOutput.h>
#include <decompiler/SDA/Symbolization/DecGraphSymbolization.h>
#include <decompiler/SDA/Optimizaton/SdaGraphFinalOptimization.h>
#include <decompiler/PCode/Decoders/DecPCodeDecoderX86.h>
#include <decompiler/PCode/DecPCodeConstValueCalc.h>
#include <decompiler/PCode/ImageAnalyzer/DecImageAnalyzer.h>

using namespace CE::Decompiler;
using namespace CE::Symbol;
using namespace CE::DataType;

namespace CE::Decompiler::Misc
{
    // show all symbols
    static std::string ShowAllSymbols(SdaCodeGraph* sdaCodeGraph) {
        std::string result;
        sdaCodeGraph->getSdaSymbols().sort([](CE::Symbol::ISymbol* a, CE::Symbol::ISymbol* b) {
            return a->getName() < b->getName();
            });

        for (auto var : sdaCodeGraph->getSdaSymbols()) {
            std::string comment = "//priority: " + std::to_string(var->getDataType()->getPriority());
            //size
            if (var->getDataType()->isArray())
                comment += ", size: " + std::to_string(var->getDataType()->getSize());
            //offsets
            if (auto localInstrSymbol = dynamic_cast<CE::Symbol::LocalInstrVarSymbol*>(var)) {
                if (!localInstrSymbol->m_instrOffsets.empty()) {
                    comment += ", offsets: ";
                    for (auto off : localInstrSymbol->m_instrOffsets) {
                        comment += std::to_string(off) + ", ";
                    }
                    comment.pop_back();
                    comment.pop_back();
                }
            }
            result += var->getDataType()->getDisplayName() + " " + var->getName() + "; " + comment + "\n";
        }
        result += "\n";
        return result;
    }

    static LinearView::BlockList* BuildBlockList(DecompiledCodeGraph* graph) {
        auto converter = LinearView::Converter(graph);
        converter.start();
        auto blockList = converter.getBlockList();
        OptimizeBlockList(blockList);
        return blockList;
    }

    /*static Symbolization::SymbolContext CreateUserSymbolDef(Project* programModule) {
        auto userSymbolDef = Symbolization::SymbolContext(programModule);
        userSymbolDef.m_globalSymbolTable = programModule->getGlobalMemoryArea();
        userSymbolDef.m_stackSymbolTable = new CE::Symbol::SymbolTable(programModule->getSymTableManager(), CE::Symbol::SymbolTable::STACK_SPACE, 100000);
        userSymbolDef.m_funcBodySymbolTable = new CE::Symbol::SymbolTable(programModule->getSymTableManager(), CE::Symbol::SymbolTable::GLOBAL_SPACE, 100000);
        return userSymbolDef;
    }*/
};