#pragma once
#include "AbstractMapper.h"
#include <list>

namespace DB
{
	//using namespace SQLite;

	class ITransaction
	{
	public:
		virtual void markAsNew(IDomainObject* obj) = 0;
		virtual void markAsDirty(IDomainObject* obj) = 0;
		virtual void markAsRemoved(IDomainObject* obj) = 0;
		virtual bool hasNewItems() = 0;
		virtual void commit() = 0;
	};

	class Transaction : public ITransaction
	{
	public:
		Transaction(Database* db);

		// add to inserted obj. list generating a new id for {obj}
		void markAsNew(IDomainObject* obj) override;

		void markAsDirty(IDomainObject* obj) override;

		void markAsRemoved(IDomainObject* obj) override;

		bool hasNewItems() override;

		void commit() override;
	private:
		Database* m_db;
		std::list<IDomainObject*> m_insertedObjs;
		std::list<IDomainObject*> m_updatedObjs;
		std::list<IDomainObject*> m_removedObjs;

		Id createSaveRecord() const;
	};
};