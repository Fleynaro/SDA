#pragma once
#include "Core/Utils/Serialization.h"

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
            utils::ISerializable* object;
        };

        Database* m_database;
        std::list<Item> m_items;
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
    };
};