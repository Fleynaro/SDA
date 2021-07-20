#pragma once
#include "AbstractType.h"
#include <list>
#include <memory>

namespace CE
{
	namespace DataType
	{
		class Unit : public IType
		{
			IType* m_type;
			std::list<int> m_levels;
		public:
			std::string m_debugName;
			
			Unit(IType* type, std::list<int> levels = {});

			Group getGroup() override;

			bool isUserDefined() override;

			bool isSystem() override {
				return m_type->isSystem();
			}

			bool isSigned() override {
				return m_type->isSigned();
			}

			IType* getBaseType(bool refType = true, bool dereferencedType = true) override {
				return m_type->getBaseType(refType, dereferencedType);
			}

			TypeManager* getTypeManager() override {
				return m_type->getTypeManager();
			}

			bool isFloatingPoint();

			int getPointerLvl();

			bool isArray();

			bool isPointer();

			std::list<int> getPointerLevels();

			void addPointerLevelInFront(int size = 1);

			void removePointerLevelOutOfFront();

			bool isString();

			bool equal(Unit* typeUnit);

			int getPriority();

			int getConversionPriority();

			const std::string getName() override;

			const std::string getComment() override;

			void setName(const std::string& name) override;

			void setComment(const std::string& comment) override;

			std::string getDisplayName() override;

			int getSize() override;

			std::string getViewValue(uint64_t value) override;

			IType* getType() const;

			static bool EqualPointerLvls(const std::list<int>& ptrList1, const std::list<int>& ptrList2);
		};
	};

	using DataTypePtr = std::shared_ptr<DataType::Unit>;

	namespace DataType
	{
		DataTypePtr GetUnit(IType* type, const std::list<int>& levels_list);
		DataTypePtr GetUnit(IType* type, const std::string& levels = "");
		std::string GetPointerLevelStr(DataTypePtr type);
		std::list<int> ParsePointerLevelsStr(const std::string& str);
		DataTypePtr CloneUnit(DataTypePtr dataType);
		DataTypePtr MakePointer(DataTypePtr dataType);
	};
};