#include "Program.h"
#include "Database/Schema.h"

using namespace sda;

Project::Project(Program* program, const std::filesystem::path& path, Context* context)
    : m_program(program), m_path(path), m_context(context)
{
    // create project directory if it doesn't exist
    if (!std::filesystem::exists(path))
        std::filesystem::create_directory(path);

    program->m_projects.push_back(std::unique_ptr<Project>(this));
    m_factory = std::make_unique<Factory>(context);
    m_database = std::make_unique<Database>(m_path / "storage.db", GetSchema());
    m_transaction = std::make_unique<Transaction>(m_database.get());
    m_changeChain = std::make_unique<ChangeChain>();
}

Program* Project::getProgram() const {
    return m_program;
}

const std::filesystem::path& Project::getPath() const {
    return m_path;
}

Context* Project::getContext() const {
    return m_context;
}

Factory* Project::getFactory() {
    return m_factory.get();
}

std::shared_ptr<ContextCallbacks> Project::getContextCallbacks() const {
    return std::dynamic_pointer_cast<ContextCallbacks>(m_context->getCallbacks());
}

Database* Project::getDatabase() const {
    return m_database.get();
}

Transaction* Project::getTransaction() const {
    return m_transaction.get();
}

ChangeChain* Project::getChangeChain() const {
    return m_changeChain.get();
}