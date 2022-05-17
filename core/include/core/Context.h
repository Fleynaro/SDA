#pragma once
#include "Object.h"

namespace sda
{
    class FunctionList;

    // Core context that contains all important entities
    class Context
    {
        std::unique_ptr<FunctionList> m_functions;
    public:
        Context();

        // Get the list of functions
        FunctionList* getFunctions();
    
        // Callbacks for the context
        class Callbacks
        {
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
        Callbacks* getCallbacks();

    private:
        std::unique_ptr<Callbacks> m_callbacks;
    };
};