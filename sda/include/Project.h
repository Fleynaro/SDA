#pragma once
#include <filesystem>
#include "Core/Context.h"
#include "Callbacks/ContextCallbacks.h"
#include "Database/Database.h"
#include "Database/Transaction.h"
#include "Change.h"

namespace sda
{
    class Program;

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

        // Get the program
        Program* getProgram() const;

        // Get path of the project
        const std::filesystem::path& getPath() const;

        // Get context of the project
        Context* getContext() const;

        // Get context callbacks
        ContextCallbacks* getContextCallbacks() const;

        // Get database of the project
        Database* getDatabase() const;

        // Get transaction of the project
        Transaction* getTransaction() const;

        // Get change chain of the project
        ChangeChain* getChangeChain() const;
    };
};