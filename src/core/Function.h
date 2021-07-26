#pragma once
#include <ghidra_sync/GhidraObject.h>
#include <datatypes/FunctionSignature.h>
#include <symbols/FunctionSymbol.h>
#include "symbols/LocalInstrVarSymbol.h"

namespace CE
{
	namespace Decompiler
	{
		class FunctionPCodeGraph;
	};

	class FunctionManager;
	class ImageDecorator;

	struct SymbolContext {
		DataType::IFunctionSignature* m_signature;
		Symbol::GlobalSymbolTable* m_globalSymbolTable;
		Symbol::StackSymbolTable* m_stackSymbolTable;
		Symbol::GlobalSymbolTable* m_funcBodySymbolTable;
		int64_t m_startOffset = 0;
	};

	class Function : public DB::DomainObject, public Ghidra::Object, public IDescription
	{
	public:
		Function(FunctionManager* manager, Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec, Symbol::StackSymbolTable* stackSymbolTable);

		SymbolContext getSymbolContext() const;

		Symbol::FunctionSymbol* getFunctionSymbol() const;

		ImageDecorator* getImage() const;

		Decompiler::FunctionPCodeGraph* getFuncGraph() const;

		const std::string getName() override;

		const std::string getComment() override;

		void setName(const std::string& name) override;

		void setComment(const std::string& comment) override;

		DataType::IFunctionSignature* getSignature() const;

		int64_t getOffset() const;

		Symbol::StackSymbolTable* getStackSymbolTable() const;

		Ghidra::Id getGhidraId() override;

		FunctionManager* getManager() const;
	private:
		ImageDecorator* m_imageDec;
		Symbol::FunctionSymbol* m_functionSymbol;
		Symbol::StackSymbolTable* m_stackSymbolTable;
		FunctionManager* m_manager;
	};
};