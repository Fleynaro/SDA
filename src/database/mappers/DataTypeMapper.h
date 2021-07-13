#pragma once
#include <database/AbstractMapper.h>
#include <datatypes/DataTypes.h>

namespace CE {
	class TypeManager;
};

namespace DB
{
	class DataTypeMapper : public AbstractMapper
	{
		bool m_loadBefore = true;
	public:
		DataTypeMapper(IRepository* repository);

		void loadBefore();

		void loadAfter();

		Id getNextId() override;

		CE::TypeManager* getManager() const;
	protected:
		IDomainObject* doLoad(Database* db, Statement& query) override;

		void doInsert(TransactionContext* ctx, IDomainObject* obj) override;

		void doUpdate(TransactionContext* ctx, IDomainObject* obj) override;

		void doRemove(TransactionContext* ctx, IDomainObject* obj) override;

	private:
		CE::DataTypePtr loadDataTypeJson(json json_dataType);

		json createDataTypeJson(CE::DataTypePtr dataType);

		void loadExtraJson(CE::DataType::UserDefinedType* userDefType, json json_extra);

		json createExtraJson(CE::DataType::UserDefinedType* userDefType);

		void bind(Statement& query, CE::DataType::UserDefinedType* userDefType);
	};
};