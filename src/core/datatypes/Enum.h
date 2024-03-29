#pragma once
#include "UserType.h"
#include <map>

namespace CE
{
	namespace DataType
	{
		class Enum : public UserDefinedType
		{
		public:
			using FieldMapType = std::map<int, std::string>;

			Enum(TypeManager* typeManager, const std::string& name, const std::string& comment = "")
				: UserDefinedType(typeManager, name, comment)
			{}

			int getSize() override;

			void setSize(int size);

			Group getGroup() override;

			const FieldMapType& getFields() const;

			void setFields(const FieldMapType& fields);

			bool removeField(int value);

			void addField(std::string name, int value);

			void deleteAll();
		private:
			FieldMapType m_fields;
			int m_size = 4;
		};
	};
};