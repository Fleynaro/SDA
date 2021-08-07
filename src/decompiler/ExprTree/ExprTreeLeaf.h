#pragma once
#include "ExprTreeNode.h"
#include <decompiler/DecSymbol.h>

namespace CE::Decompiler::ExprTree
{
	class ILeaf : public virtual INode
	{};

	class ISymbolLeaf : public ILeaf, public IStoragePathNode
	{};

	class SymbolLeaf : public Node, public ISymbolLeaf, public PCode::IRelatedToInstruction
	{
	public:
		Symbol::Symbol* m_symbol;

		SymbolLeaf(Symbol::Symbol* symbol);

		int getSize() override;

		HS getHash() override;

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override;

		INode* clone(NodeCloneContext* ctx) override;

		StoragePath getStoragePath() override {
			StoragePath path;
			if (const auto regVar = dynamic_cast<Symbol::RegisterVariable*>(m_symbol)) {
				path.m_storage = Storage(Storage::STORAGE_REGISTER, regVar->m_register.getGenericId());
			}
			else if (const auto localVar = dynamic_cast<Symbol::LocalVariable*>(m_symbol)) {
				if (!localVar->m_isTemp) {
					path.m_storage = Storage(Storage::STORAGE_REGISTER, localVar->m_register.getGenericId());
				}
			}
			else if (const auto funcVar = dynamic_cast<Symbol::FunctionResultVar*>(m_symbol)) {
				path.m_storage = Storage(Storage::STORAGE_REGISTER, funcVar->m_register.getGenericId());
			}
			return path;
		}
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
		friend class ExprTreeViewGenerator;
		uint64_t m_value;
		int m_size;
	public:

		NumberLeaf(uint64_t value, int size);

		NumberLeaf(double value, int size);

		uint64_t getValue() override;

		void setValue(uint64_t value) override;

		int getSize() override;

		INode* clone(NodeCloneContext* ctx) override;
	};

	class FloatNanLeaf : public Node
	{
	public:
		FloatNanLeaf();

		int getSize() override;

		HS getHash() override;

		INode* clone(NodeCloneContext* ctx) override;

		bool isFloatingPoint() override;
	};
};