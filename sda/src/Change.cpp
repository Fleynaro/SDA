#include "SDA/Change.h"
#include "SDA/Factory.h"

using namespace sda;

void ChangeList::undo() {
    for (auto it = m_changes.rbegin(); it != m_changes.rend(); ++it) {
        (*it)->undo();
    }
}

void ChangeList::add(std::unique_ptr<IChange> change) {
    m_changes.push_back(std::move(change));
}

ObjectChange* ChangeList::getOrCreateObjectChange(IFactory* factory) {
    if(!m_changes.empty())
        if(auto objectChange = dynamic_cast<ObjectChange*>(m_changes.back().get()))
            return objectChange;
            
    // if no object change is found, create one
    auto objectChange = new ObjectChange(factory);
    m_changes.push_back(std::unique_ptr<IChange>(objectChange));
    return objectChange;
}

void ChangeChain::undo() {
    m_changePointsIt--;
    if (!m_changePointsIt->isVisited)
        m_isUndoing = true;

    m_changePointsIt->changeList.undo();

    if (!m_changePointsIt->isVisited)
        m_isUndoing = false;
    m_changePointsIt->isVisited = true;
}

void ChangeChain::redo() {
    m_changePointsIt->changeListBack.undo();
    m_changePointsIt++;
}

bool ChangeChain::isAtBeginning() const {
    return m_changePointsIt == m_changePoints.begin();
}

bool ChangeChain::isAtEnd() const {
    return m_changePointsIt == m_changePoints.end();
}

void ChangeChain::newChangeList() {
    m_changePoints.erase(m_changePointsIt, m_changePoints.end());
    m_changePoints.emplace_back();
    m_changePointsIt = m_changePoints.end();
}

ChangeList* ChangeChain::getChangeList() const {
    if (m_changePointsIt == m_changePoints.end())
        return nullptr;
    if (m_isUndoing) {
        if (m_changePointsIt->isVisited)
            return nullptr;
        return &m_changePointsIt->changeListBack;
    }
    return &std::prev(m_changePointsIt)->changeList;
}

ObjectChange::ObjectChange(IFactory* factory)
    : m_factory(factory)
{}

void ObjectChange::undo() {
    std::list<std::pair<utils::ISerializable*, boost::json::object*>> objectsToDeserialize;
    std::list<utils::IDestroyable*> objectsToDestroy;

    for (auto it = m_changes.rbegin(); it != m_changes.rend(); ++it) {
        switch (it->type)
        {
        case ObjectChangeData::New:
            if (auto object = dynamic_cast<utils::IDestroyable*>(it->object)) {
                objectsToDestroy.push_back(object);
            } else {
                throw std::runtime_error("Object is not destroyable");
            }
            break;

        case ObjectChangeData::Modified:
            objectsToDeserialize.emplace_back(it->object, &it->initState);
            break;

        case ObjectChangeData::Removed:
            auto object = m_factory->create(it->initState);
            objectsToDeserialize.emplace_back(object, &it->initState);
            break;
        }
    }

    // serialize objects
    for(auto& [object, initState] : objectsToDeserialize)
        object->deserialize(*initState);

    // destroy objects
    for(auto object : objectsToDestroy)
        object->destroy();
}

void ObjectChange::markAsNew(utils::ISerializable* obj) {
    m_changes.push_back({ObjectChangeData::New, obj});
    m_affectedObjects.insert(obj);
}

void ObjectChange::markAsModified(utils::ISerializable* obj) {
    if(m_affectedObjects.find(obj) != m_affectedObjects.end())
        return;
    // if object is not affected yet
    boost::json::object initState;
    obj->serialize(initState);
    m_changes.push_back({ObjectChangeData::Modified, obj, initState});
    m_affectedObjects.insert(obj);
}

void ObjectChange::markAsRemoved(utils::ISerializable* obj) {
    boost::json::object initState;
    obj->serialize(initState);
    m_changes.push_back({ObjectChangeData::Removed, nullptr, initState});
    m_affectedObjects.erase(obj);
}