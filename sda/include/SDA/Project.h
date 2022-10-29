#pragma once
#include <filesystem>
// include context (TODO: rewrite, problem in unique_ptr)
#include "SDA/Core/Context.h"
#include "SDA/Core/Image/AddressSpace.h"
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/Symbol/Symbol.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"
// include context END
#include "SDA/Factory.h"
#include "SDA/Callbacks/ContextCallbacks.h"
#include "SDA/Database/Database.h"
#include "SDA/Database/Transaction.h"
#include "SDA/Change.h"

namespace sda
{
    class Program;

    class Project
    {
        Program* m_program;
        std::filesystem::path m_path;
        std::unique_ptr<Context> m_context;
        std::unique_ptr<Factory> m_factory;
        std::unique_ptr<Database> m_database;
        std::unique_ptr<Transaction> m_transaction;
        std::unique_ptr<ChangeChain> m_changeChain;
    public:
        Project(Program* program, const std::filesystem::path& path, std::unique_ptr<Context> context);

        // Get the program
        Program* getProgram() const;

        // Get path of the project
        const std::filesystem::path& getPath() const;

        // Get context of the project
        Context* getContext() const;

        // Get factory
        Factory* getFactory();

        // Get context callbacks
        std::shared_ptr<ContextCallbacks> getContextCallbacks() const;

        // Get database of the project
        Database* getDatabase() const;

        // Get transaction of the project
        Transaction* getTransaction() const;

        // Get change chain of the project
        ChangeChain* getChangeChain() const;
    };
};