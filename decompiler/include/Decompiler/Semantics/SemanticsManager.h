#pragma once
#include <list>
#include <map>
#include "Semantics.h"
#include "SemanticsObject.h"
#include "SemanticsPropagator.h"

namespace sda::decompiler
{
    class SemanticsManager
    {
        friend class SemanticsObject;
        friend class SemanticsPropagator;
        Context* m_context;
        std::map<SemanticsObject::Id, std::unique_ptr<SemanticsObject>> m_objects;
        std::list<std::unique_ptr<SemanticsPropagator>> m_propagators;
    public:
        SemanticsManager(Context* context);

        Context* getContext() const;
        
        void removeObject(SemanticsObject* object, SemanticsContextOperations& operations);

        SemanticsObject* getObject(SemanticsObject::Id id) const;

        template<typename T>
        T* getObject(SemanticsObject::Id id) const {
            return dynamic_cast<T*>(getObject(id));
        }

        void removeSemantics(Semantics* semantics, SemanticsContextOperations& operations);
        
        bool isSimiliarityConsidered() const;

        void propagate(SemanticsContextOperations& operations);
    };
};