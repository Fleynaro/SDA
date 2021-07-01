#pragma once
#include <utilities/Description.h>
#include <database/DomainObject.h>

namespace CE
{
	class TypeManager;
};

namespace CE::DataType
{
	class IType : virtual public IDescription
	{
	protected:
		virtual ~IType() {}
	public:
		enum Group
		{
			Simple,
			Enum,
			Structure,
			Class,
			Typedef,
			FunctionSignature
		};

		virtual Group getGroup() = 0;

		virtual std::string getDisplayName() = 0;

		virtual int getSize() = 0;

		virtual bool isUserDefined() = 0;

		virtual bool isSystem() = 0;

		virtual bool isSigned() = 0;

		virtual IType* getBaseType(bool refType = true, bool dereferencedType = true) = 0;

		virtual std::string getViewValue(uint64_t value) = 0;

		virtual TypeManager* getTypeManager() = 0;
	};

	class AbstractType : virtual public IType, public Description
	{
	public:
		AbstractType(TypeManager* typeManager, const std::string& name, const std::string& comment = "")
			: m_typeManager(typeManager), Description(name, comment)
		{}

		IType* getBaseType(bool refType = true, bool dereferencedType = true) override;

		bool isSystem() override;

		bool isSigned() override;

		std::string getViewValue(uint64_t value) override;

		void setTypeManager(TypeManager* typeManager);

		TypeManager* getTypeManager() override;
	private:
		TypeManager* m_typeManager;
	};
};