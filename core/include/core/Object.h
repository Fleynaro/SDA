#pragma once
#include <map>
#include <memory>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Serialization.h"

namespace sda
{
    using ObjectId = boost::uuids::uuid;
    class IContext;
    class Context;

    // Interface for all domain objects
    class IObject
    {
    public:
        // Get the unique identifier of the object
        virtual ObjectId getId() const = 0;

        // Get the name of the object
        virtual const std::string& getName() const = 0;

        // Set the name of the object
        virtual void setName(const std::string& name) = 0;

        // Get the comment of the object
        virtual const std::string& getComment() const = 0;

        // Set the comment of the object
        virtual void setComment(const std::string& comment) = 0;

        // Bind the object to the context
        virtual void bind(IContext* context) = 0;
    };

    // Base class for all domain objects
    class Object : public virtual IObject, public virtual ISerializable
    {
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            boost::serialization::base_object<ISerializable>(*this);
            ar & m_id;
            ar & m_name;
            ar & m_comment;
        }    
    
        ObjectId m_id;
        std::string m_name = "";
        std::string m_comment = "";
    public:
        Object();

        // Get the unique identifier of the object
        ObjectId getId() const;

        // Get the name of the object
        const std::string& getName() const;

        // Set the name of the object
        void setName(const std::string& name);

        // Get the comment of the object
        const std::string& getComment() const;

        // Set the comment of the object
        void setComment(const std::string& comment);
    };

    BOOST_SERIALIZATION_ASSUME_ABSTRACT(Object)

    // Base class for all domain object's lists
    template<typename T = IObject>
    class ObjectList
    {
        using ObjectListType = std::map<ObjectId, std::unique_ptr<T>>;
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

        ObjectList() = default;

        // Add an object to the list
        void add(std::unique_ptr<T> object) {
            m_objects[object->getId()] = std::move(object);
        }

        // Get an object by its unique identifier
        T* get(ObjectId id) {
            auto it = m_objects.find(id);
            if (it == m_objects.end()) {
                return nullptr;
            }
            return it->second.get();
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