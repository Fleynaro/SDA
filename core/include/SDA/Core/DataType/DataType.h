#pragma once
#include <unordered_map>
#include "SDA/Core/Object/ObjectList.h"

namespace sda
{
    enum class ScalarType {
        UnsignedInt,
        SignedInt,
        FloatingPoint
    };

    class PointerDataType;
    class ArrayDataType;

    class DataType : public ContextObject
    {
    protected:
        DataType(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        void notifyModified(Object::ModState state) override;

    public:
        static inline const std::string Class = "data_type";

        PointerDataType* getPointerTo();

        ArrayDataType* getArrayOf(const std::list<size_t>& dimensions);

        virtual DataType* getBaseType();

        virtual bool isVoid() const;

        virtual bool isPointer() const;

        virtual bool isScalar(ScalarType type) const;

        virtual size_t getSize() const = 0;

        void serialize(boost::json::object& data) const override;

        void destroy() override;
    };

    class ScalarDataType;

    class DataTypeList : public ObjectList<DataType>
    {
    private:
        std::unordered_map<std::string, DataType*> m_dataTypes;
        std::unordered_map<size_t, ScalarDataType*> m_scalarDataTypes;

    public:
        using ObjectList<DataType>::ObjectList;

        void initDefault();

        DataType* getByName(const std::string& name) const;

        ScalarDataType* getScalar(ScalarType type, size_t size);

        void notifyModified(DataType* dataType, Object::ModState state);

    private:
        void onObjectAdded(DataType* dataType) override;

        void onObjectRemoved(DataType* dataType) override;

        size_t scalarToHash(ScalarType type, size_t size) const;
    };
};