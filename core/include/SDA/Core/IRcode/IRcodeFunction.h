#pragma once
#include <list>
#include "IRcodeBlock.h"
#include "SDA/Core/Pcode/PcodeFunctionGraph.h"
#include "SDA/Core/DataType/SignatureDataType.h"
#include "SDA/Core/Symbol/FunctionSymbol.h"

namespace sda::ircode
{
    class Program;
    class Function
    {
        friend class Block;
        Program* m_program = nullptr;
        pcode::FunctionGraph* m_functionGraph = nullptr;
        std::map<pcode::Block*, Block> m_blocks;
        utils::BitSet m_varIds;
    public:
        Function(Program* program, pcode::FunctionGraph* functionGraph);

        std::string getName();

        Program* getProgram() const;

        pcode::FunctionGraph* getFunctionGraph() const;

        FunctionSymbol* getFunctionSymbol() const;
        
        Offset getEntryOffset() const;

        Block* getEntryBlock();

        std::map<pcode::Block*, Block>& getBlocks();

        Block* toBlock(pcode::Block* pcodeBlock);

        std::list<std::shared_ptr<Variable>> getVariables();

        std::shared_ptr<Variable> findVariableById(size_t id);
    };
};