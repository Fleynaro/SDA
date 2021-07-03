#pragma once
#include "GhidraSync.h"

namespace CE::Ghidra
{
	class GhidraObjectLoader
	{
	public:
		GhidraObjectLoader(CE::Project* programModule);

		~GhidraObjectLoader();

		void analyse() const;

		std::list<IObject*>& getObjectsToUpsert();

		std::list<IObject*>& getObjectsToRemove();
	private:
		CE::Project* m_project;
		std::list<IObject*> m_upsertedObjs;
		std::list<IObject*> m_removedObjs;
	};
};