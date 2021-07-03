#pragma once
#include <datatypes/TypeUnit.h>

namespace CE
{
	class SymbolManager;

	namespace Symbol
	{
		enum Type {
			FUNCTION = 1,
			GLOBAL_VAR,
			LOCAL_INSTR_VAR,
			LOCAL_STACK_VAR,
			FUNC_PARAMETER,
			STRUCT_FIELD
		};

		class ISymbol : virtual public IDescription
		{
		public:
			virtual bool isAutoSymbol() = 0;

			virtual Type getType() = 0;

			virtual DataTypePtr getDataType() = 0;

			virtual void setDataType(DataTypePtr dataType) = 0;

			virtual int getSize() {
				return getDataType()->getSize();
			}
		};

		class AbstractSymbol : virtual public ISymbol, public DB::DomainObject, public Description
		{
		public:
			AbstractSymbol(SymbolManager* manager, DataTypePtr dataType, const std::string& name, const std::string& comment = "")
				: m_manager(manager), m_dataType(dataType), Description(name, comment)
			{}

			void setAutoSymbol(bool toggle);

			bool isAutoSymbol() override;

			SymbolManager* getManager() const;

			DataTypePtr getDataType() override;

			void setDataType(DataTypePtr dataType) override;
		private:
			DataTypePtr m_dataType;
			SymbolManager* m_manager;
			bool m_isAutoSymbol = false;
		};
	};
};