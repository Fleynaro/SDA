#pragma once
#include "Core/Object.h"

namespace sda
{
    class Database;

    class Transaction
    {
        struct Item {
            enum {
                New,
                Modified,
                Removed
            } type;
            ISerializable* object;
        };

        Database* m_database;
        std::list<Item> m_items;
    public:
        Transaction(Database* database);

        // Mark an object as new
        void markAsNew(ISerializable* obj);

        // Mark an object as modified
        void markAsModified(ISerializable* obj);

        // Mark an object as removed
        void markAsRemoved(ISerializable* obj);

        // Commit the transaction
        void commit();
    };
};