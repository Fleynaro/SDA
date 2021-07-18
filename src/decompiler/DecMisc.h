#pragma once
#include <decompiler/Decompiler.h>
#include <decompiler/LinearView/DecLinearViewOptimization.h>
#include <decompiler/SDA/Symbolization/DecGraphSymbolization.h>
#include "decompiler/LinearView/DecLinearView.h"
#include <decompiler/PCode/ImageAnalyzer/DecImageAnalyzer.h>

namespace CE::Decompiler::Misc
{
    extern LinearView::BlockList* BuildBlockList(DecompiledCodeGraph* graph);

    /*static Symbolization::SymbolContext CreateUserSymbolDef(Project* programModule) {
        auto userSymbolDef = Symbolization::SymbolContext(programModule);
        userSymbolDef.m_globalSymbolTable = programModule->getGlobalMemoryArea();
        userSymbolDef.m_stackSymbolTable = new CE::Symbol::SymbolTable(programModule->getSymTableManager(), CE::Symbol::SymbolTable::STACK_SPACE, 100000);
        userSymbolDef.m_funcBodySymbolTable = new CE::Symbol::SymbolTable(programModule->getSymTableManager(), CE::Symbol::SymbolTable::GLOBAL_SPACE, 100000);
        return userSymbolDef;
    }*/
};