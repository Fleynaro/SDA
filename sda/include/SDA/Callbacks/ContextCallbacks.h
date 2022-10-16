#pragma once
#include "SDA/Core/Context.h"

namespace sda
{
    class Project;
    class ObjectChange;

    class ContextCallbacks : public Context::Callbacks
    {
        std::list<std::unique_ptr<Callbacks>> m_callbacks;
        Project* m_project;
    public:
        ContextCallbacks(Context* context, Project* project);

        // Add a callback to the list
        void add(std::unique_ptr<Callbacks> callback);

        // Called when an object is added to the context
        void onObjectAdded(Object* obj) override;

        // Called when an object is modified in the context
        void onObjectModified(Object* obj) override;

        // Called when an object is removed from the context
        void onObjectRemoved(Object* obj) override;

    private:
        // Get or create object change
        ObjectChange* getOrCreateObjectChange() const;
    };
};