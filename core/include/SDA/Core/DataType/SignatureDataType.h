#pragma once
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/Symbol/FunctionParameterSymbol.h"
#include "SDA/Core/Platform/CallingConvention.h"

namespace sda
{
    class FunctionSymbol;
    class SignatureDataType : public DataType
    {
        std::shared_ptr<CallingConvention> m_callingConvention;
        std::vector<FunctionParameterSymbol*> m_parameters;
        DataType* m_returnType;
        CallingConvention::Map m_storages;
        bool m_updateStorages = true;
    public:
        static inline const std::string Type = "signature";

        SignatureDataType(
            Context* context,
            std::shared_ptr<CallingConvention> callingConvention,
            Object::Id* id = nullptr,
            const std::string& name = "",
            DataType* returnType = nullptr,
            const std::vector<FunctionParameterSymbol*>& parameters = {});
    
        std::list<FunctionSymbol*> getFunctionSymbols() const;

        std::shared_ptr<CallingConvention> getCallingConvention() const;

        const CallingConvention::Map& getStorages();

        const CallingConvention::StorageInfo* findStorageInfo(const CallingConvention::Storage& storage);

        // TODO: to change the parameters, use setParameters() only (like in react js)
        void setParameters(const std::vector<FunctionParameterSymbol*>& parameters);

        const std::vector<FunctionParameterSymbol*>& getParameters() const;

        void setReturnType(DataType* returnType);

        DataType* getReturnType() const;

        size_t getSize() const override;

        void clear();

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};