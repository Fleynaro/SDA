#pragma once
#include <ghidra_sync/GhidraObject.h>
#include <datatypes/FunctionSignature.h>
#include <symbols/FunctionSymbol.h>
#include <ImageDecorator.h>

namespace CE
{
	namespace Decompiler
	{
		class FunctionPCodeGraph;
	};

	class FunctionManager;

	struct SymbolContext {
		DataType::IFunctionSignature* m_signature;
		CE::Symbol::SymbolTable* m_globalSymbolTable;
		CE::Symbol::SymbolTable* m_stackSymbolTable;
		CE::Symbol::SymbolTable* m_funcBodySymbolTable;
		int64_t m_startOffset = 0;
	};

	class Function : public DB::DomainObject, public Ghidra::Object, public IDescription
	{
	public:
		Function(FunctionManager* manager, Symbol::FunctionSymbol* functionSymbol, ImageDecorator* imageDec, Symbol::SymbolTable* stackSymbolTable)
			: m_manager(manager), m_functionSymbol(functionSymbol), m_imageDec(imageDec), m_stackSymbolTable(stackSymbolTable)
		{
			functionSymbol->setFunction(this);
		}

		SymbolContext getSymbolContext() {
			SymbolContext symbolCtx;
			symbolCtx.m_signature = getSignature();
			symbolCtx.m_globalSymbolTable = m_imageDec->getGlobalSymbolTable();
			symbolCtx.m_funcBodySymbolTable = m_imageDec->getFuncBodySymbolTable();
			symbolCtx.m_stackSymbolTable = m_stackSymbolTable;
			return symbolCtx;
		}

		Symbol::FunctionSymbol* getFunctionSymbol();

		ImageDecorator* getImage();

		Decompiler::FunctionPCodeGraph* getFuncGraph();

		const std::string getName() override;

		const std::string getComment() override;

		void setName(const std::string& name) override;

		void setComment(const std::string& comment) override;

		DataType::IFunctionSignature* getSignature();

		int64_t getOffset();

		Symbol::SymbolTable* getStackSymbolTable();

		Ghidra::Id getGhidraId() override;

		FunctionManager* getManager();
	private:
		ImageDecorator* m_imageDec;
		Symbol::FunctionSymbol* m_functionSymbol;
		Symbol::SymbolTable* m_stackSymbolTable;
		FunctionManager* m_manager;
	};
};