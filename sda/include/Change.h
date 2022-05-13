#pragma once
#include "Core/Object.h"

namespace sda
{
    // Change interface
    class IChange
    {
    public:
        // Cancel changes
        virtual void undo() = 0;

        // Apply canceled changes
        virtual void redo() = 0;
    };

    // Change list stores changes and can undo and redo them in one go
    class ChangeList : public IChange
    {
        std::list<std::unique_ptr<IChange>> m_changes;
    public:
        // Cancel all changes
        void undo() override;

        // Apply all canceled changes
        void redo() override;

        // Add new change
        void add(std::unique_ptr<IChange> change);
    };

    // Change chain stores changes and can undo and redo them one by one
    class ChangeChain : public IChange
    {
        std::list<ChangeList> m_changes;
        std::list<ChangeList>::iterator m_curChangeIt;
    public:
        // Move to the previous change list
        void undo() override;

        // Move to the next change list
        void redo() override;

        // Create a new change list
        void newChangeList();

        // Get the current change list
        ChangeList* getChangeList();
    };

    // Object change stores changes of objects (new, modified, removed)
    class ObjectChange : public IChange
    {

    public:
        // Cancel object changes
        void undo() override;

        // Apply canceled object changes
        void redo() override;

        // Mark an object as new
        void markAsNew(ISerializable* obj);

        // Mark an object as modified
        void markAsModified(ISerializable* obj);

        // Mark an object as removed
        void markAsRemoved(ISerializable* obj);
    };
};