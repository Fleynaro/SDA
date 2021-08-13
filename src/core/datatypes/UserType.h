#pragma once
#include "TypeUnit.h"

namespace CE::DataType
{
	class IUserDefinedType : virtual public IType, virtual public DB::IDomainObject
	{};

	class UserDefinedType : public AbstractType, public DB::DomainObject, virtual public IUserDefinedType
	{
	public:
		UserDefinedType(TypeManager* typeManager, const std::string& name, const std::string& comment = "")
			: AbstractType(typeManager, name, comment)
		{}

		bool isUserDefined() override;

		std::string getDisplayName() override;
	};
};