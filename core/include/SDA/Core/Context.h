#pragma once
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
        Platform* m_platform;
        std::unique_ptr<AddressSpaceList> m_addressSpaces;
        std::unique_ptr<ImageList> m_images;
        std::unique_ptr<DataTypeList> m_dataTypes;
        std::unique_ptr<SymbolList> m_symbols;
        std::unique_ptr<SymbolTableList> m_symbolTables;
    public:
        Context(Platform* platform);

        ~Context();

        // Init context with default objects
        void initDefault();

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
    
        // Callbacks for the context
        class Callbacks {
            std::shared_ptr<Callbacks> m_prevCallbacks;
            bool m_enabled = true;
        public:
            virtual std::string getName() const;

            // Called when an object is added to the context
            void onObjectAdded(Object* obj);

            // Called when an object is modified in the context
            void onObjectModified(Object* obj);

            // Called when an object is removed from the context
            void onObjectRemoved(Object* obj);

            // Called when context is destroyed
            void onContextDestroyed(Context* context);

            void setPrevCallbacks(std::shared_ptr<Callbacks> prevCallbacks);

            void setEnabled(bool enabled);

            static std::shared_ptr<Callbacks> Find(const std::string& name, std::shared_ptr<Callbacks> callbacks);

        protected:
            virtual void onObjectAddedImpl(Object* obj) {};

            virtual void onObjectModifiedImpl(Object* obj) {};

            virtual void onObjectRemovedImpl(Object* obj) {};

            virtual void onContextDestroyedImpl(Context* context) {};
        };

        // Set the callbacks for the context
        void setCallbacks(std::shared_ptr<Callbacks> callbacks);

        // Get the callbacks for the context
        std::shared_ptr<Callbacks> getCallbacks() const;

    private:
        std::shared_ptr<Callbacks> m_callbacks;
    };
};