#include "Database/Schema.h"
#include "Core/Image/AddressSpace.h"
#include "Core/Image/Image.h"
#include "Core/DataType/DataType.h"
#include "Core/Symbol/Symbol.h"
#include "Core/Symbol/SymbolTable.h"

using namespace sda;

std::unique_ptr<Schema> sda::GetSchema() {
    std::list<Schema::Collection> collections = {
        { AddressSpace::GetCollectionName() },
        { Image::GetCollectionName() },
        { DataType::GetCollectionName() },
        { Symbol::GetCollectionName() },
        { SymbolTable::GetCollectionName() }
    };
    return std::make_unique<Schema>(collections);
}