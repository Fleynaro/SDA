#include "Program.h"
#include "Database/Transaction.h"
#include "Database/Schema.h"
#include "Change.h"

using namespace sda;

Project::Project(Program* program, const std::filesystem::path& path, Context* context)
    : m_program(program), m_path(path), m_context(context)
{
    // create project directory if it doesn't exist
    if (!std::filesystem::exists(path))
        std::filesystem::create_directory(path);

    program->m_projects.push_back(std::unique_ptr<Project>(this));
    m_database = std::make_unique<Database>(m_path / "storage.db", GetSchema());
    m_transaction = std::make_unique<Transaction>(m_database.get());
    m_changeChain = std::make_unique<ChangeChain>(this);
}

const std::filesystem::path& Project::getPath() const {
    return m_path;
}

Context* Project::getContext() {
    return m_context;
}

ContextCallbacks* Project::getContextCallbacks() {
    return dynamic_cast<ContextCallbacks*>(m_context->getCallbacks());
}

Database* Project::getDatabase() {
    return m_database.get();
}

Transaction* Project::getTransaction() {
    return m_transaction.get();
}

ChangeChain* Project::getChangeChain() {
    return m_changeChain.get();
}