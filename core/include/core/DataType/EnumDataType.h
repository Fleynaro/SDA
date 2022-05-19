#pragma once
#include "Core/DataType/DataType.h"

namespace sda
{
    class EnumDataType : public DataType
    {
        using Key = int;

        std::map<Key, std::string> m_fields;
    public:
        static inline const std::string Type = "enum";

        EnumDataType(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        // todo: to change the fields, use setFields() only (like in react js)
        void setFields(const std::map<Key, std::string>& fields);

        const std::map<Key, std::string>& getFields() const;

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};