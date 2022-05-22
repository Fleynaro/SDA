#pragma once
#include "Core/Object/ContextObject.h"

namespace sda
{
    class AddressSpaceList;
    class ImageList;
    class DataTypeList;
    class SymbolList;
    class SymbolTableList;

    // Core context that contains all important entities
    class Context
    {
        std::unique_ptr<AddressSpaceList> m_addressSpaces;
        std::unique_ptr<ImageList> m_images;
        std::unique_ptr<DataTypeList> m_dataTypes;
        std::unique_ptr<SymbolList> m_symbols;
        std::unique_ptr<SymbolTableList> m_symbolTables;
    public:
        Context();

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
    
        // Callbacks for the context
        class Callbacks {
        public:
            // Called when an object is added to the context
            virtual void onObjectAdded(ContextObject* obj) {}

            // Called when an object is modified in the context
            virtual void onObjectModified(ContextObject* obj) {}

            // Called when an object is removed from the context
            virtual void onObjectRemoved(ContextObject* obj) {}
        };

        // Set the callbacks for the context
        std::unique_ptr<Callbacks> setCallbacks(std::unique_ptr<Callbacks> callbacks);

        // Get the callbacks for the context
        Callbacks* getCallbacks() const;

    private:
        std::unique_ptr<Callbacks> m_callbacks;
    };
};