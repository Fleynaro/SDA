#include "DecMisc.h"

// show all symbols
std::string Misc::ShowAllSymbols(SdaCodeGraph* sdaCodeGraph) {
    std::string result;
    sdaCodeGraph->getSdaSymbols().sort([](ISymbol* a, ISymbol* b) {
        return a->getName() < b->getName();
        });

    for (auto var : sdaCodeGraph->getSdaSymbols()) {
        std::string comment = "//priority: " + std::to_string(var->getDataType()->getPriority());
        //size
        if (var->getDataType()->isArray())
            comment += ", size: " + std::to_string(var->getDataType()->getSize());
        //offsets
        if (auto localInstrSymbol = dynamic_cast<LocalInstrVarSymbol*>(var)) {
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

LinearView::BlockList* Misc::BuildBlockList(DecompiledCodeGraph* graph) {
    auto converter = LinearView::Converter(graph);
    converter.start();
    const auto blockList = converter.getBlockList();
    OptimizeBlockList(blockList);
    return blockList;
}
