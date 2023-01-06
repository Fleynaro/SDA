#include "SDA/Core/Context.h"
#include "SDA/Core/Image/AddressSpace.h"
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/Symbol/Symbol.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"

using namespace sda;

Context::Context(Platform* platform)
    : m_platform(platform)
{
    m_addressSpaces = std::make_unique<AddressSpaceList>(this);
    m_images = std::make_unique<ImageList>(this);
    m_dataTypes = std::make_unique<DataTypeList>(this);
    m_symbols = std::make_unique<SymbolList>(this);
    m_symbolTables = std::make_unique<SymbolTableList>(this);
    m_callbacks = std::make_unique<Callbacks>();
}

void Context::initDefault() {
    m_dataTypes->initDefault();
}

Platform* Context::getPlatform() const {
    return m_platform;
}

AddressSpaceList* Context::getAddressSpaces() const {
    return m_addressSpaces.get();
}

ImageList* Context::getImages() const {
    return m_images.get();
}

DataTypeList* Context::getDataTypes() const {
    return m_dataTypes.get();
}

SymbolList* Context::getSymbols() const {
    return m_symbols.get();
}

SymbolTableList* Context::getSymbolTables() const {
    return m_symbolTables.get();
}

void Context::setCallbacks(std::shared_ptr<Callbacks> callbacks) {
    m_callbacks = callbacks;
}

std::shared_ptr<Context::Callbacks> Context::getCallbacks() const {
    return m_callbacks;
}

std::string Context::Callbacks::getName() const {
    return "Empty";
}

void Context::Callbacks::onObjectAdded(Object* obj) {
    if (m_prevCallbacks) {
        m_prevCallbacks->onObjectAdded(obj);
    }
    if (m_enabled) {
        onObjectAddedImpl(obj);
    }
}

void Context::Callbacks::onObjectModified(Object* obj) {
    if (m_prevCallbacks) {
        m_prevCallbacks->onObjectModified(obj);
    }
    if (m_enabled) {
        onObjectModifiedImpl(obj);
    }
}

void Context::Callbacks::onObjectRemoved(Object* obj) {
    if (m_prevCallbacks) {
        m_prevCallbacks->onObjectRemoved(obj);
    }
    if (m_enabled) {
        onObjectRemovedImpl(obj);
    }
}

void Context::Callbacks::setPrevCallbacks(std::shared_ptr<Context::Callbacks> callbacks) {
    m_prevCallbacks = callbacks;
}

void Context::Callbacks::setEnabled(bool enabled) {
    m_enabled = enabled;
}

std::shared_ptr<Context::Callbacks> Context::Callbacks::Find(const std::string& name, std::shared_ptr<Context::Callbacks> callbacks) {
    if (callbacks->getName() == name)
        return callbacks;
    if (callbacks->m_prevCallbacks)
        return Find(name, callbacks->m_prevCallbacks);
    return nullptr;
}
