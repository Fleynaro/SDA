#pragma once
#include "SDA/Core/Context.h"

namespace sda
{
    class Project;
    class ObjectChange;

    class ProjectContextCallbacks : public Context::Callbacks
    {
        Project* m_project;
        bool m_transactionEnabled = true;
        bool m_changeEnabled = true;
    public:
        ProjectContextCallbacks(Project* project);

        std::string getName() const override;

        // Called when an object is added to the context
        void onObjectAdded(Object* obj) override;

        // Called when an object is modified in the context
        void onObjectModified(Object* obj) override;

        // Called when an object is removed from the context
        void onObjectRemoved(Object* obj) override;

        // Enable or disable transaction
        void setTransactionEnabled(bool enabled) { m_transactionEnabled = enabled; }

        // Enable or disable change
        void setChangeEnabled(bool enabled) { m_changeEnabled = enabled; }

    private:
        // Get or create object change
        ObjectChange* getOrCreateObjectChange() const;
    };
};