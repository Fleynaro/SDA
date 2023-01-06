#include "SDA/Program.h"
#include "SDA/Database/Schema.h"
#include "SDA/Database/Loader.h"

using namespace sda;

Project::Project(Program* program, const std::filesystem::path& path, std::unique_ptr<Context> context)
    : m_program(program), m_path(path), m_context(std::move(context))
{
    // create project directory if it doesn't exist
    if (!std::filesystem::exists(path))
        std::filesystem::create_directory(path);

    program->m_projects.push_back(std::unique_ptr<Project>(this));
    program->getCallbacks()->onProjectAdded(this);
    m_factory = std::make_unique<Factory>(getContext());
    m_database = std::make_unique<Database>(m_path / "storage.db", GetSchema());
    m_transaction = std::make_unique<Transaction>(getDatabase());
    m_changeChain = std::make_unique<ChangeChain>();
    
    // set project callbacks
    auto transactionCallbacks = std::make_shared<TransactionContextCallbacks>(getTransaction());
    transactionCallbacks->setPrevCallbacks(getContext()->getCallbacks());
    auto changeChainCallbacks = std::make_shared<ChangeChainContextCallbacks>(getChangeChain(), getFactory());
    changeChainCallbacks->setPrevCallbacks(transactionCallbacks);
    getContext()->setCallbacks(changeChainCallbacks);
}

Program* Project::getProgram() const {
    return m_program;
}

const std::filesystem::path& Project::getPath() const {
    return m_path;
}

Context* Project::getContext() const {
    return m_context.get();
}

Factory* Project::getFactory() {
    return m_factory.get();
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

void Project::load() {
    getDatabase()->init();
    Loader loader(getDatabase(), getFactory());
    loader.load();
}

void Project::save() {
    getTransaction()->commit();
}

bool Project::canBeSaved() const {
    return !getTransaction()->isEmpty();
}