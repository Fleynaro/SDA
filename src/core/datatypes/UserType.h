#pragma once
#include "TypeUnit.h"
#include <ghidra_sync/GhidraObject.h>

namespace CE::DataType
{
	class IUserDefinedType : virtual public IType, virtual public DB::IDomainObject, virtual public Ghidra::IObject
	{};

	class UserDefinedType : public AbstractType, public DB::DomainObject, public Ghidra::Object, public IUserDefinedType
	{
	public:
		UserDefinedType(TypeManager* typeManager, const std::string& name, const std::string& comment = "")
			: AbstractType(typeManager, name, comment)
		{}

		bool isUserDefined() override;

		std::string getDisplayName() override;

		Ghidra::Id getGhidraId() override;
	};
};