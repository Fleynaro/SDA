#include "DecMisc.h"

LinearView::BlockList* Misc::BuildBlockList(DecompiledCodeGraph* graph) {
    auto converter = LinearView::Converter(graph);
    converter.start();
    const auto blockList = converter.getBlockList();
    LinearView::OptimizeBlockList(blockList);
    return blockList;
}
