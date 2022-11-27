#pragma once
#include "SDA/Core/Context.h"

namespace sda
{
    class Project;
    class ObjectChange;

    class ProjectContextCallbacks : public Context::Callbacks
    {
        Project* m_project;
        std::shared_ptr<Callbacks> m_prevCallbacks;
    public:
        ProjectContextCallbacks(Project* project, std::shared_ptr<Callbacks> prevCallbacks);

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