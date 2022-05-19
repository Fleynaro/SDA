#pragma once
#include "Core/DataType/DataType.h"
#include "Core/Symbol/FunctionParameterSymbol.h"

namespace sda
{
    class SignatureDataType : public DataType
    {
        std::vector<FunctionParameterSymbol*> m_parameters;
        DataType* m_returnType;
    public:
        static inline const std::string Type = "signature";

        SignatureDataType(Context* context, Object::Id* id = nullptr, const std::string& name = "");

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