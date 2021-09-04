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
		std::vector<Symbol::FuncParameterSymbol*> m_params;
	public:
		bool m_areParamStoragesChanged = false;
		IFunctionSignature* m_funcSignature;
		ParameterList(IFunctionSignature* funcSignature)
			: m_funcSignature(funcSignature)
		{}
		
		Symbol::FuncParameterSymbol* operator[](int idx) {
			return m_params[idx];
		}
		
		int getParamsCount() const {
			return static_cast<int>(m_params.size());
		}
		
		void addParameter(Symbol::FuncParameterSymbol* param) {
			param->m_paramIdx = static_cast<int>(m_params.size()) + 1;
			param->m_signature = m_funcSignature;
			m_params.push_back(param);
			m_areParamStoragesChanged = true;
		}

		void removeParameter(int idx) {
			m_params.erase(m_params.begin() + idx - 1);
			for (int i = idx - 1; i < getParamsCount(); i++) {
				m_params[i]->m_paramIdx--;
			}
			m_areParamStoragesChanged = true;
		}

		void moveParameter(int idx, int dir) {
			const auto newParamIdx = idx + dir;
			if (newParamIdx <= 0 || newParamIdx >= m_params.size() + 1)
				return;
			std::swap(m_params[idx - 1]->m_paramIdx, m_params[newParamIdx - 1]->m_paramIdx);
			std::swap(m_params[idx - 1], m_params[newParamIdx - 1]);
		}

		void clear() {
			m_params.clear();
		}
	};
	
	using StoragesList = std::list<std::pair<int, Decompiler::Storage>>;

	class IFunctionSignature : virtual public IUserDefinedType
	{
	public:
		virtual bool isAuto() = 0;

		virtual CallingConvetion getCallingConvetion() = 0;

		virtual StoragesList& getCustomStorages() = 0;

		virtual std::string getSigName() = 0;

		virtual void setReturnType(DataTypePtr returnType) = 0;

		virtual DataTypePtr getReturnType() = 0;

		virtual ParameterList& getParameters() = 0;

		virtual void addParameter(const std::string& name, DataTypePtr dataType, const std::string& comment = "") = 0;

		virtual Decompiler::FunctionCallInfo getCallInfo() = 0;

		virtual void updateParameterStorages() = 0;

		virtual IFunctionSignature* clone() = 0;

		virtual void apply(IFunctionSignature* funcSignature) = 0;
	};

	class FunctionSignature : public UserDefinedType, public IFunctionSignature
	{
	public:
		FunctionSignature(TypeManager* typeManager, const std::string& name, const std::string& comment = "", CallingConvetion callingConvetion = FASTCALL);

		~FunctionSignature() {
			for (int i = 0; i < m_parameters.getParamsCount(); i++) {
				delete m_parameters[i];
			}
		}
		
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

		ParameterList& getParameters() override;

		void addParameter(const std::string& name, DataTypePtr dataType, const std::string& comment = "") override;

		Decompiler::FunctionCallInfo getCallInfo() override;

		void updateParameterStorages() override;

		IFunctionSignature* clone() override;

		void apply(IFunctionSignature* funcSignature) override;

	private:
		bool m_isAuto = false;
		bool m_isReturnStorageChanged = false;
		CallingConvetion m_callingConvetion;
		std::list<Decompiler::ParameterInfo> m_paramInfos;
		std::list<std::pair<int, Decompiler::Storage>> m_customStorages;
		ParameterList m_parameters;
		DataTypePtr m_returnType;
	};
};