#include "GhidraObjectLoader.h"
#include <managers/FunctionManager.h>

using namespace CE;
using namespace Ghidra;

GhidraObjectLoader::GhidraObjectLoader(Project* programModule)
	: m_project(programModule)
{}

GhidraObjectLoader::~GhidraObjectLoader() {
	for (auto it : m_removedObjs) {
		delete it;
	}
}

void GhidraObjectLoader::analyse() const
{
	const std::string query_text = "\
		SELECT id,deleted,sync_id,type FROM (\
			SELECT def_id,deleted,ghidra_sync_id,1 FROM sda_func_defs\
		) AS t1 INNER JOIN sda_ghidra_sync AS t2 ON t1.sync_id = t2.sync_id INNER JOIN sda_saves AS t3 ON t2.date < t3.date";
	Statement query(m_project->getDB(), query_text);

	while (query.executeStep())
	{
		DB::Id obj_id = query.getColumn("id");
		const int isDeleted = query.getColumn("deleted");
		switch (static_cast<int>(query.getColumn("type")))
		{
		case 1:
			if (isDeleted) {

			} else {

			}
			break;
		}
	}
}

std::list<IObject*>& GhidraObjectLoader::getObjectsToUpsert() {
	return m_upsertedObjs;
}

std::list<IObject*>& GhidraObjectLoader::getObjectsToRemove() {
	return m_removedObjs;
}
