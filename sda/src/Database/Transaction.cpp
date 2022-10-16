#include "SDA/Database/Transaction.h"
#include "SDA/Database/Database.h"

using namespace sda;

Transaction::Transaction(Database* database)
    : m_database(database)
{}

void Transaction::markAsNew(utils::ISerializable* obj) {
    if (m_objects.find(obj) != m_objects.end())
        throw std::runtime_error("Object already marked as new");
    m_objects[obj] = New;
}

void Transaction::markAsModified(utils::ISerializable* obj) {
    auto it = m_objects.find(obj);
    if (it != m_objects.end()) {
        if (it->second == Removed)
            throw std::runtime_error("Object already marked as removed");
        return;
    }
    m_objects[obj] = Modified;
}

void Transaction::markAsRemoved(utils::ISerializable* obj) {
    auto it = m_objects.find(obj);
    if (it != m_objects.end()) {
        if (it->second == New) {
            // the object is new, so we can just remove it from the transaction and don't save it
            m_objects.erase(it);
            return;
        }
    }
    m_objects[obj] = Removed;
}

void Transaction::commit() {
    for(auto& [obj, changeType] : m_objects) {
        boost::json::object data;
        obj->serialize(data);
        if (data["temporary"].get_bool())
            continue;
        auto collectionName = std::string(data["collection"].get_string().c_str());
        auto collection = m_database->getCollection(collectionName);
        if (changeType == New || changeType == Modified) {
            collection->write(data);
        } else if (changeType == Removed) {
            collection->remove(data);
        }
    }

    m_objects.clear();
}