#pragma once
#include <set>
#include <map>
#include "SDA/Core/Context.h"
#include "SDA/Core/Utils/Serialization.h"

namespace sda
{
    class Database;

    class Transaction
    {
        enum ChangeType {
            New,
            Modified,
            Removed
        };

        Database* m_database;
        std::map<utils::ISerializable*, ChangeType> m_objects;
    public:
        Transaction(Database* database);

        // Mark an object as new
        void markAsNew(utils::ISerializable* obj);

        // Mark an object as modified
        void markAsModified(utils::ISerializable* obj);

        // Mark an object as removed
        void markAsRemoved(utils::ISerializable* obj);

        // Commit the transaction
        void commit();

        bool isEmpty() const;
    };

    class TransactionContextCallbacks : public Context::Callbacks
    {
        Transaction* m_transaction;
    public:
        TransactionContextCallbacks(Transaction* transaction);

        std::string getName() const override;

        // Called when an object is added to the context
        void onObjectAddedImpl(Object* obj) override;

        // Called when an object is modified in the context
        void onObjectModifiedImpl(Object* obj) override;

        // Called when an object is removed from the context
        void onObjectRemovedImpl(Object* obj) override;
    };
};