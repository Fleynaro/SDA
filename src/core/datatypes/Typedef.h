#pragma once
#include "UserType.h"

namespace CE::DataType
{
	class Typedef : public UserDefinedType
	{
	public:
		Typedef(TypeManager* typeManager, const std::string& name, const std::string& comment = "");

		Group getGroup() override;

		int getSize() override;

		std::string getViewValue(uint64_t value) override;

		void setRefType(DataTypePtr refType);

		DataTypePtr getRefType() const;
	private:
		DataTypePtr m_refType;
	};
};