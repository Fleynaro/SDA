#pragma once
#include "GhidraSync.h"

namespace CE::Ghidra
{
	class SyncCommitment
	{
	public:
		SyncCommitment(Sync* sync);

		void upsert(IObject* obj);

		void remove(IObject* obj);

		void commit();
		
	private:
		Sync* m_sync;
		std::list<IObject*> m_upsertedObjs;
		std::list<IObject*> m_removedObjs;

		int createSyncRecord();
	};
};