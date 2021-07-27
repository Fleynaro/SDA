#pragma once
#include "UserType.h"
#include <symbols/FuncParameterSymbol.h>
#include <decompiler/DecStorage.h>
#include <vector>

namespace CE::DataType
{
	class IFunctionSignature : public IUserDefinedType
	{
	public:
		enum CallingConvetion {
			FASTCALL
		};

		virtual bool isAuto() = 0;

		virtual CallingConvetion getCallingConvetion() = 0;

		virtual std::list<std::pair<int, Decompiler::Storage>>& getCustomStorages() = 0;

		virtual std::string getSigName() = 0;

		virtual void setReturnType(DataTypePtr returnType) = 0;

		virtual DataTypePtr getReturnType() = 0;

		virtual std::vector<Symbol::FuncParameterSymbol*>& getParameters() = 0;

		virtual void addParameter(Symbol::FuncParameterSymbol* symbol) = 0;

		// todo: remove
		virtual void addParameter(const std::string& name, DataTypePtr dataType, const std::string& comment = "") = 0;

		// todo: remove
		virtual void removeLastParameter() = 0;

		virtual void deleteAllParameters() = 0;

		virtual Decompiler::FunctionCallInfo getCallInfo() = 0;
	};

	class FunctionSignature : public UserDefinedType, public IFunctionSignature
	{
	public:
		FunctionSignature(TypeManager* typeManager, const std::string& name, const std::string& comment = "", CallingConvetion callingConvetion = FASTCALL);
			
		Group getGroup() override;

		int getSize() override;

		std::string getDisplayName() override;

		bool isAuto() override {
			return m_isAuto;
		}

		void setAuto(bool toggle) {
			m_isAuto = toggle;
		}

		CallingConvetion getCallingConvetion() override;

		std::list<std::pair<int, Decompiler::Storage>>& getCustomStorages() override;

		std::string getSigName() override;

		void setReturnType(DataTypePtr returnType) override;

		DataTypePtr getReturnType() override;

		std::vector<Symbol::FuncParameterSymbol*>& getParameters() override;

		void addParameter(Symbol::FuncParameterSymbol* symbol) override;

		void addParameter(const std::string& name, DataTypePtr dataType, const std::string& comment = "") override;

		void removeLastParameter() override;

		void deleteAllParameters() override;

		Decompiler::FunctionCallInfo getCallInfo() override;

	private:
		bool m_isAuto = false;
		CallingConvetion m_callingConvetion;
		std::list<Decompiler::ParameterInfo> m_paramInfos;
		bool m_hasSignatureUpdated = false;
		std::list<std::pair<int, Decompiler::Storage>> m_customStorages;
		std::vector<Symbol::FuncParameterSymbol*> m_parameters;
		DataTypePtr m_returnType;

		void updateParameterStorages();
	};
};