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
        Context* m_context;
        std::map<SemanticsObject::Id, std::unique_ptr<SemanticsObject>> m_objects;
        std::list<std::unique_ptr<Semantics>> m_semantics;
        std::list<std::unique_ptr<SemanticsPropagator>> m_propagators;
    public:
        SemanticsManager(Context* context);

        Context* getContext() const;
        
        SemanticsObject* addObject(std::unique_ptr<SemanticsObject> object);

        SemanticsObject* getObject(SemanticsObject::Id id) const;

        template<typename T>
        T* getObject(SemanticsObject::Id id) const {
            return dynamic_cast<T*>(getObject(obj));
        }

        Semantics* addSemantics(std::unique_ptr<Semantics> semantics);
        
        void addPropagator(std::unique_ptr<SemanticsPropagator> propagator);

        bool isSimiliarityConsidered() const;
    };
};