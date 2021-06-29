#pragma once
#include <DB/AbstractMapper.h>
#include <Image/ImageDecorator.h>
#include <Decompiler/Graph/DecPCodeGraph.h>

namespace CE {
	class ImageManager;
};

namespace DB
{
	class ImageMapper : public AbstractMapper
	{
	public:
		ImageMapper(IRepository* repository);

		void loadAll();

		Id getNextId() override;

		CE::ImageManager* getManager();
	protected:
		IDomainObject* doLoad(Database* db, SQLite::Statement& query) override;

		void doInsert(TransactionContext* ctx, IDomainObject* obj) override;

		void doUpdate(TransactionContext* ctx, IDomainObject* obj) override;

		void doRemove(TransactionContext* ctx, IDomainObject* obj) override;
		
	private:
		void decodePCodeBlock(CE::Decompiler::PCodeBlock* block, CE::ImageDecorator* imageDec);

		void loadFuncPCodeGraphJson(const json& json_func_graph, CE::Decompiler::FunctionPCodeGraph* funcGraph, CE::ImageDecorator* imageDec);

		json createFuncPCodeGraphJson(CE::Decompiler::FunctionPCodeGraph* funcPCodeGraph);

		void bind(SQLite::Statement& query, CE::ImageDecorator* imageDec);
	};
};