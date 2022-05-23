#pragma once
#include "Core/Object/ObjectList.h"

namespace sda
{
    class DataType : public ContextObject
    {
    public:
        static inline const std::string Collection = "data_types";

        DataType(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        virtual size_t getSize() const = 0;

        void serialize(boost::json::object& data) const override;

        void destroy() override;
    };

    class ScalarDataType;

    class DataTypeList : public ObjectList<DataType>
    {
    public:
        struct ScalarInfo {
            bool isFloatingPoint;
            bool isSigned;
            size_t size;

            bool operator<(const ScalarInfo& other) const;
        };

    private:
        std::map<std::string, DataType*> m_dataTypes;
        std::map<ScalarInfo, ScalarDataType*> m_scalarDataTypes;

    public:
        using ObjectList<DataType>::ObjectList;

        DataType* getByName(const std::string& name) const;

        ScalarDataType* getScalar(bool isFloatingPoint, bool isSigned, size_t size);

    private:
        void onObjectAdded(DataType* dataType) override;

        void onObjectRemoved(DataType* dataType) override;
    };
};