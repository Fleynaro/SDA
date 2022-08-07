#pragma once
#include "Core/DataType/DataType.h"
#include "Core/Symbol/FunctionParameterSymbol.h"
#include "Core/Platform/CallingConvention.h"

namespace sda
{
    class SignatureDataType : public DataType
    {
        std::shared_ptr<CallingConvention> m_callingConvention;
        std::vector<FunctionParameterSymbol*> m_parameters;
        DataType* m_returnType;
    public:
        static inline const std::string Type = "signature";

        SignatureDataType(
            Context* context,
            std::shared_ptr<CallingConvention> callingConvention,
            Object::Id* id = nullptr,
            const std::string& name = "");

        std::shared_ptr<CallingConvention> getCallingConvention() const;

        CallingConvention::Map getStorages() const;

        // todo: to change the parameters, use setParameters() only (like in react js)
        void setParameters(const std::vector<FunctionParameterSymbol*>& parameters);

        const std::vector<FunctionParameterSymbol*>& getParameters() const;

        void setReturnType(DataType* returnType);

        DataType* getReturnType() const;

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};