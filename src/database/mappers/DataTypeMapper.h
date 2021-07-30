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
		void loadExtraJson(CE::DataType::UserDefinedType* userDefType, json json_extra);

		json createExtraJson(CE::DataType::UserDefinedType* userDefType);

		void loadParamsListJson(json json_params, CE::DataType::ParameterList& params);

		json createParamsListJson(CE::DataType::ParameterList& params);

		void bind(Statement& query, CE::DataType::UserDefinedType* userDefType);
	};

	CE::DataTypePtr DeserializeDataType(json json_dataType, CE::TypeManager* manager);

	json SerializeDataType(CE::DataTypePtr dataType);
};