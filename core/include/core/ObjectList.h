#pragma once
#include "Context.h"

namespace sda
{
    // Base class for all domain object's lists
    template<typename T = IObject>
    class ObjectList
    {
        using ObjectListType = std::map<ObjectId, std::unique_ptr<T>>;
        Context* m_context;
        ObjectListType m_objects;
    public:
        // Iterator wrapper over the map
        struct Iterator 
        {
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using reference = T*;

            Iterator(const typename ObjectListType::iterator& it)
                : m_it(it)
            {}

            reference operator*() {
                return m_it->second.get();
            }

            Iterator& operator++() {
                ++m_it;
                return *this;
            }

            Iterator operator++(int) {
                Iterator it = *this;
                ++(*this);
                return it;
            }

            bool operator==(const Iterator& it) const {
                return m_it == it.m_it;
            }

            bool operator!=(const Iterator& it) const {
                return !(*this == it);
            }

        private:
            typename ObjectListType::iterator m_it;
        };

        ObjectList(Context* context)
            : m_context(context)
        {}

        // Add an object to the list
        void add(std::unique_ptr<T> object) {
            m_context->getCallbacks()->onObjectAdded(object.get());
            m_objects[object->getId()] = std::move(object);
        }

        // Remove an object from the list
        void remove(const ObjectId& uuid) {
            auto it = m_objects.find(uuid);
            if (it == m_objects.end())
                throw std::runtime_error("Object not found");
            m_context->getCallbacks()->onObjectRemoved(it->second.get());
            m_objects.erase(it);
        }

        // Get an object by its unique identifier
        T* get(const ObjectId& uuid) {
            auto it = m_objects.find(uuid);
            if (it == m_objects.end()) {
                return nullptr;
            }
            return it->second.get();
        }

        // Get an object by its string unique identifier
        T* get(const std::string& uuid) {
            return get(boost::uuids::string_generator()(uuid));
        }

        // Get iterator to the first object
        Iterator begin() {
            return Iterator(m_objects.begin());
        }

        // Get iterator to the last object
        Iterator end() {
            return Iterator(m_objects.end());
        }
    };
};