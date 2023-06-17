#pragma once
#include "SDA/Core/Event/EventPipe.h"
#include "SDA/Core/Object/ContextObject.h"
#include "SDA/Core/Platform/Platform.h"

namespace sda
{
    class AddressSpaceList;
    class ImageList;
    class DataTypeList;
    class SymbolList;
    class SymbolTableList;
    // see also Core/ContextInclude.h

    // Core context that contains all important entities
    class Context
    {
        std::shared_ptr<EventPipe> m_eventPipe;
        Platform* m_platform;
        std::unique_ptr<AddressSpaceList> m_addressSpaces;
        std::unique_ptr<ImageList> m_images;
        std::unique_ptr<DataTypeList> m_dataTypes;
        std::unique_ptr<SymbolList> m_symbols;
        std::unique_ptr<SymbolTableList> m_symbolTables;
    public:
        Context(std::shared_ptr<EventPipe> eventPipe, Platform* platform);

        ~Context();

        // Init context with default objects
        void initDefault();

        std::shared_ptr<EventPipe> getEventPipe() const;

        // Get the platform (e.g. x86, arm, etc.)
        Platform* getPlatform() const;

        // Get the list of address spaces
        AddressSpaceList* getAddressSpaces() const;

        // Get the list of images
        ImageList* getImages() const;

        // Get the list of data types
        DataTypeList* getDataTypes() const;

        // Get the list of symbols
        SymbolList* getSymbols() const;

        // Get the list of symbol tables
        SymbolTableList* getSymbolTables() const;

        // Allows to optimize event handling by avoiding multiple same events within commit
        static std::shared_ptr<EventPipe> CreateOptimizedEventPipe();

        // Allows to track which property of an object has changed
        static std::shared_ptr<EventPipe> CreatePropertyEventPipe(const std::function<size_t(Object*)>& propertyHasher);
    };
};