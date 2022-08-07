#pragma once
#include "PcodeInstruction.h"

namespace sda::pcode
{
    class Render
    {
        const RegisterHelper* m_regHelper;
        size_t m_commentingCounter = 0;
    public:
        Render(const RegisterHelper* regHelper);

        virtual void renderInstruction(const Instruction* instruction) const;

        virtual void renderVarnode(const Varnode* varnode, bool renderSizeAndOffset = true) const;
        
        void commenting(bool toggle);

    protected:
        enum class Token {
            Mnemonic,
            Register,
            VirtRegister,
            Number,
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
        StreamRender(std::ostream& output, const RegisterHelper* regHelper);

    protected:
        void renderTokenImpl(const std::string& text, Token token) const override;
    };
};