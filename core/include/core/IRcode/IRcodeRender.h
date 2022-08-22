#pragma once
#include "IRcodeOperation.h"
#include "IRcodeDataTypeProvider.h"
#include "Core/Pcode/PcodeRender.h"

namespace sda::ircode
{
    class Render
    {
        pcode::Render* m_pcodeRender;
        size_t m_commentingCounter = 0;
        bool m_extendInfo = false;
        DataTypeProvider* m_dataTypeProvider;
    public:
        Render(pcode::Render* pcodeRender);

        void setDataTypeProvider(DataTypeProvider* dataTypeProvider);

        virtual void renderOperation(Operation* operation);

        virtual void renderValue(const Value* value, bool extended = false) const;

        virtual void renderLinearExpr(const LinearExpression* linearExpr) const;

        void commenting(bool toggle);

        void setExtendInfo(bool toggle);

    protected:
        enum class Token {
            Operation,
            Variable,
            Comment,
            Other
        };

        void renderToken(const std::string& text, Token token) const;

        virtual void renderTokenImpl(const std::string& text, Token token) const = 0;
    };

    class StreamRender : public Render
    {
        std::ostream& m_output;
    public:
        StreamRender(std::ostream& output, pcode::Render* pcodeRender);

    protected:
        void renderTokenImpl(const std::string& text, Token token) const override;
    };
};