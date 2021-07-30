#pragma once
#include "UserType.h"
#include <symbols/FuncParameterSymbol.h>
#include <decompiler/DecStorage.h>
#include <vector>

namespace CE::DataType
{
	enum CallingConvetion {
		FASTCALL
	};

	class IFunctionSignature;
	class ParameterList
	{
		IFunctionSignature* m_funcSignature;
		std::vector<Symbol::FuncParameterSymbol> m_params;
		std::list<Decompiler::ParameterInfo> m_paramInfos;
		DataTypePtr m_returnType;
	public:
		CallingConvetion m_callingConvetion;
		std::list<std::pair<int, Decompiler::Storage>> m_customStorages;
		
		ParameterList(IFunctionSignature* funcSignature = nullptr, CallingConvetion callingConvetion = CallingConvetion::FASTCALL, DataTypePtr returnType = nullptr)
			: m_funcSignature(funcSignature), m_callingConvetion(callingConvetion), m_returnType(returnType)
		{}

		CallingConvetion getCallingConvetion() const {
			return m_callingConvetion;
		}

		Symbol::FuncParameterSymbol* operator[](int idx) {
			return &m_params[idx];
		}

		int getParamsCount() const {
			return static_cast<int>(m_params.size());
		}

		Symbol::FuncParameterSymbol createParameter(const std::string& name, DataTypePtr dataType,
		                                            const std::string& comment = "");

		void addParameter(Symbol::FuncParameterSymbol param) {
			param.m_paramIdx = static_cast<int>(m_params.size()) + 1;
			m_params.push_back(param);
		}

		void removeParameter(int idx) {
			m_params.erase(m_params.begin() + idx - 1);
			for(int i = idx - 1; i < getParamsCount(); i ++) {
				m_params[i].m_paramIdx--;
			}
		}

		void moveParameter(int idx, int dir) {
			const auto newParamIdx = idx + dir;
			if (newParamIdx <= 0 || newParamIdx >= m_params.size() + 1)
				return;
			std::swap(m_params[idx - 1], m_params[newParamIdx - 1]);
		}

		void setReturnType(DataTypePtr returnType) {
			m_returnType = returnType;
		}

		DataTypePtr getReturnType() const {
			return m_returnType;
		}

		Decompiler::FunctionCallInfo getCallInfo() const {
			return m_paramInfos;
		}

		void updateParameterStorages();
	};
	
	class IFunctionSignature : virtual public IUserDefinedType
	{
	public:
		virtual bool isAuto() = 0;

		virtual std::string getSigName() = 0;

		virtual ParameterList& getParameters() = 0;

		virtual void setParameters(const ParameterList& params) = 0;

		virtual void addParameter(const std::string& name, DataTypePtr dataType, const std::string& comment = "") = 0;

		virtual void setReturnType(DataTypePtr returnType) = 0;
	};

	class FunctionSignature : public UserDefinedType, public IFunctionSignature
	{
		ParameterList m_paramsList;
		bool m_isAuto = false;
	public:
		FunctionSignature(TypeManager* typeManager, const std::string& name, const std::string& comment = "", CallingConvetion callingConvetion = FASTCALL);
			
		Group getGroup() override;

		int getSize() override;

		std::string getDisplayName() override;

		bool isAuto() override;

		void setAuto(bool toggle);

		std::string getSigName() override;

		ParameterList& getParameters() override;

		void setParameters(const ParameterList& params) override;

		void addParameter(const std::string& name, DataTypePtr dataType, const std::string& comment = "") override {
			m_paramsList.addParameter(m_paramsList.createParameter(name, dataType, comment));
			m_paramsList.updateParameterStorages();
		}

		void setReturnType(DataTypePtr returnType) {
			m_paramsList.setReturnType(returnType);
			m_paramsList.updateParameterStorages();
		}
	};
};