#include "Core/Context.h"
#include "Core/Image/AddressSpace.h"
#include "Core/Image/Image.h"
#include "Core/DataType/DataType.h"
#include "Core/Symbol/Symbol.h"
#include "Core/SymbolTable/SymbolTable.h"

using namespace sda;

Context::Context(std::unique_ptr<Platform> platform)
    : m_platform(std::move(platform))
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
    return m_platform.get();
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