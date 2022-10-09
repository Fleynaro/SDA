#pragma once
#include <set>
#include "Core/Utils/Serialization.h"

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
    };
};