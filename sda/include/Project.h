#pragma once
#include <filesystem>
#include "Core/Context.h"
#include "Callbacks/ContextCallbacks.h"

namespace sda
{
    class Program;
    class Database;
    class Transaction;
    class ChangeChain;

    class Project
    {
        Program* m_program;
        std::filesystem::path m_path;
        Context* m_context;
        std::unique_ptr<Database> m_database;
        std::unique_ptr<Transaction> m_transaction;
        std::unique_ptr<ChangeChain> m_changeChain;
    public:
        Project(Program* program, const std::filesystem::path& path, Context* context);

        // Get path of the project
        const std::filesystem::path& getPath() const;

        // Get context of the project
        Context* getContext();

        // Get context callbacks
        ContextCallbacks* getContextCallbacks();

        // Get database of the project
        Database* getDatabase();

        // Get transaction of the project
        Transaction* getTransaction();

        // Get change chain of the project
        ChangeChain* getChangeChain();
    };
};