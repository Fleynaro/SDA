#include "Core/Image/ImageContext.h"
#include "Core/Symbol/SymbolTable.h"

using namespace sda;

ImageContext::ImageContext(Context* context, Object::Id* id, const std::string& name)
    : ContextObject(context, id, name)
{
    m_context->getImageContexts()->add(std::unique_ptr<ImageContext>(this));
}

SymbolTable* ImageContext::getGlobalSymbolTable() const {
    return m_globalSymbolTable;
}

SymbolTable* ImageContext::getFuncBodySymbolTable() const {
    return m_funcBodySymbolTable;
}

void ImageContext::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);

    data["global_symbol_table"] = m_globalSymbolTable->serializeId();
    data["func_body_symbol_table"] = m_funcBodySymbolTable->serializeId();
}

void ImageContext::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);

    m_globalSymbolTable = m_context->getSymbolTables()->get(data["global_symbol_table"]);
    m_funcBodySymbolTable = m_context->getSymbolTables()->get(data["func_body_symbol_table"]);
}

void ImageContext::destroy() {
    m_context->getImageContexts()->remove(this);
}