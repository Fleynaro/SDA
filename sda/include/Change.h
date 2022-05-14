#pragma once
#include <set>
#include "Core/Serialization.h"
#include "Core/Destroy.h"

namespace sda
{
    // Change interface
    class IChange
    {
    public:
        // Cancel changes
        virtual void undo() = 0;
    };

    class ObjectChange;
    class IContext;
    class IFactory;

    // Change list stores changes and can undo them in one go
    class ChangeList : public IChange
    {
        std::list<std::unique_ptr<IChange>> m_changes;
    public:
        // Cancel all changes
        void undo() override;

        // Add new change
        void add(std::unique_ptr<IChange> change);

        // Get or create object change
        ObjectChange* getOrCreateObjectChange(IContext* context, IFactory* factory);
    };

    // Change chain stores change points
    class ChangeChain
    {
        struct ChangePoint {
            ChangeList changeList;
            ChangeList changeListBack;
            bool isVisited = false;
        };
        std::list<ChangePoint> m_changePoints;
        std::list<ChangePoint>::iterator m_changePointsIt = m_changePoints.end();
        bool m_isUndoing = false;
    public:
        // Move to the previous change point
        void undo();

        // Move to the next change point
        void redo();

        // Check if we are at the beginning of the change chain
        bool isAtBeginning() const;

        // Check if we are at the end of the change chain
        bool isAtEnd() const;

        // Create a new change list
        void newChangeList();

        // Get the current change list
        ChangeList* getChangeList() const;
    };

    // Object change stores changes of objects (new, modified, removed)
    class ObjectChange : public IChange
    {
        struct ObjectChangeData {
            enum {
                New,
                Modified,
                Removed
            } type;
            ISerializable* object; 
            boost::json::object initState;
        };

        IContext* m_context;
        IFactory* m_factory;
        std::list<ObjectChangeData> m_changes;
        std::set<ISerializable*> m_affectedObjects;
    public:
        ObjectChange(IContext* context, IFactory* factory);

        // Cancel object changes
        void undo() override;
        
        // Mark an object as new
        void markAsNew(ISerializable* obj);

        // Mark an object as modified
        void markAsModified(ISerializable* obj);

        // Mark an object as removed
        void markAsRemoved(ISerializable* obj);
    };
};