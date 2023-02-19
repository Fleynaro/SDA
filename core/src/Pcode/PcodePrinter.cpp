#include "SDA/Core/Pcode/PcodePrinter.h"
#include "SDA/Core/Pcode/PcodeGraph.h"
#include "SDA/Core/Utils/IOManip.h"
#include <sstream>
#include "rang.hpp"

using namespace sda::pcode;

Printer::Printer(const RegisterRepository* regRepo, Graph* graph)
    : m_regRepo(regRepo), m_graph(graph)
{}

std::string Printer::Print(const Instruction* instruction, const RegisterRepository* regRepo) {
    Printer printer(regRepo);
    std::stringstream ss;
    printer.setOutput(ss);
    printer.printInstruction(instruction);
    return ss.str();
}

void Printer::printFunctionGraph(FunctionGraph* functionGraph) {
    auto blockInfos = functionGraph->getBlocks();
    // sort blocks by level and offset
    blockInfos.sort([](const FunctionGraph::BlockInfo& a, const FunctionGraph::BlockInfo& b) {
        if (a.level != b.level)
            return a.level < b.level;
        return a.block->getMinOffset() < b.block->getMinOffset();
    });
    // print blocks
    for (auto& [block, level] : blockInfos) {
        printBlock(block, level);
        if (block != blockInfos.back().block)
            newLine();
    }
}

void Printer::printBlock(pcode::Block* block, size_t level) {
    printToken("Block ", SYMBOL);
    printToken(block->getName(), IDENTIFIER);
    printToken("(level: ", SYMBOL);
    printToken(std::to_string(level), NUMBER);
    if (block->getNearNextBlock()) {
        printToken(", near: ", SYMBOL);
        printToken(block->getNearNextBlock()->getName(), IDENTIFIER);
    }
    if (block->getFarNextBlock()) {
        printToken(", far: ", SYMBOL);
        printToken(block->getFarNextBlock()->getName(), IDENTIFIER);
    }
    printToken("):", SYMBOL);
    startBlock();
    newLine();
    for (const auto& [offset, instruction] : block->getInstructions()) {
        printInstruction(instruction);
        if (instruction != block->getInstructions().rbegin()->second)
            newLine();
    }
    endBlock();
}

void Printer::printInstruction(const Instruction* instruction) const {
    if (instruction->getOutput()) {
        printVarnode(instruction->getOutput());
        printToken(" = ", SYMBOL);
    }

    printToken(magic_enum::enum_name(instruction->getId()).data(), MNEMONIC);

    if (instruction->getInput0()) {
        printToken(" ", SYMBOL);
        printVarnode(instruction->getInput0());
    }
    if (instruction->getInput1()) {
        printToken(", ", SYMBOL);
        printVarnode(instruction->getInput1());
    }
}

void Printer::printVarnode(std::shared_ptr<Varnode> varnode, bool printSizeAndOffset) const {
    if (auto regVarnode = std::dynamic_pointer_cast<RegisterVarnode>(varnode)) {
        const auto& reg = regVarnode->getRegister();
        auto regStr = reg.toString(m_regRepo, printSizeAndOffset);
        if (reg.getRegType() == Register::Virtual) {
            printToken(regStr, VIRT_REGISTER);
        } else {
            printToken(regStr, REGISTER);
        }
    }
    else if (auto constVarnode = std::dynamic_pointer_cast<ConstantVarnode>(varnode)) {
        if (m_graph && constVarnode->isAddress()) {
            auto block = m_graph->getBlockAt(constVarnode->getValue());
            if (block) {
                printToken("<", SYMBOL);
                printToken(block->getName(), IDENTIFIER);
                printToken(">", SYMBOL);
                std::stringstream ss;
                ss << ":" << constVarnode->getSize();
                printToken(ss.str(), NUMBER);
                return;
            }
        }
        std::stringstream ss;
        ss << "0x" << utils::to_hex() << constVarnode->getValue();
        if (printSizeAndOffset) 
            ss << ":" << constVarnode->getSize();
        printToken(ss.str(), NUMBER);
    }
}