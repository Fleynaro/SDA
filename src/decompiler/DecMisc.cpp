#include "DecMisc.h"

using namespace CE::Decompiler;
using namespace CE::Symbol;
using namespace CE::DataType;

LinearView::BlockList* Misc::BuildBlockList(DecompiledCodeGraph* graph) {
    auto converter = LinearView::Converter(graph);
    converter.start();
    const auto blockList = converter.getBlockList();
    LinearView::OptimizeBlockList(blockList);
    return blockList;
}
