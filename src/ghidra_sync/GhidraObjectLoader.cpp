#include "GhidraObjectLoader.h"
#include <Manager/FunctionManager.h>

using namespace CE;
using namespace CE::Ghidra;

GhidraObjectLoader::GhidraObjectLoader(CE::Project* programModule)
	: m_project(programModule)
{}

GhidraObjectLoader::~GhidraObjectLoader() {
	for (auto it : m_removedObjs) {
		delete it;
	}
}

void GhidraObjectLoader::analyse() {
	std::string query_text = "\
		SELECT id,deleted,sync_id,type FROM (\
			SELECT def_id,deleted,ghidra_sync_id,1 FROM sda_func_defs\
		) AS t1 INNER JOIN sda_ghidra_sync AS t2 ON t1.sync_id = t2.sync_id INNER JOIN sda_saves AS t3 ON t2.date < t3.date";
	SQLite::Statement query(m_project->getDB(), query_text);

	while (query.executeStep())
	{
		DB::Id obj_id = query.getColumn("id");
		int isDeleted = query.getColumn("deleted");
		switch ((int)query.getColumn("type"))
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
