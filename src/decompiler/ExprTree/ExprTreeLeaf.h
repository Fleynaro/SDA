#pragma once
#include "ExprTreeNode.h"
#include <decompiler/DecSymbol.h>

namespace CE::Decompiler::ExprTree
{
	class ILeaf : public virtual INode
	{};

	class SymbolLeaf : public Node, public ILeaf, public PCode::IRelatedToInstruction
	{
	public:
		Symbol::Symbol* m_symbol;

		SymbolLeaf(Symbol::Symbol* symbol);

		int getSize() override;

		HS getHash() override;

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override;

		INode* clone(NodeCloneContext* ctx) override;

		std::string printDebug() override;
	};

	class INumberLeaf : public ILeaf
	{
	public:
		virtual uint64_t getValue() = 0;

		virtual void setValue(uint64_t value) = 0;

		HS getHash() override;
	};

	class NumberLeaf : public Node, public INumberLeaf
	{
		uint64_t m_value;
		int m_size;
	public:

		NumberLeaf(uint64_t value, int size);

		NumberLeaf(double value, int size);

		uint64_t getValue() override;

		void setValue(uint64_t value) override;

		int getSize() override;

		INode* clone(NodeCloneContext* ctx) override;

		std::string printDebug() override;
	};

	class FloatNanLeaf : public Node
	{
	public:
		FloatNanLeaf();

		int getSize() override;

		HS getHash() override;

		INode* clone(NodeCloneContext* ctx) override;

		bool isFloatingPoint() override;

		std::string printDebug() override;
	};
};