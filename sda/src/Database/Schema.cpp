#include "SDA/Database/Schema.h"
#include "SDA/Core/Image/AddressSpace.h"
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/Symbol/Symbol.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"

using namespace sda;

std::unique_ptr<Schema> sda::GetSchema() {
    std::list<Schema::Collection> collections = {
        { AddressSpace::Collection },
        { Image::Collection },
        { DataType::Collection },
        { Symbol::Collection },
        { SymbolTable::Collection }
    };
    return std::make_unique<Schema>(collections);
}