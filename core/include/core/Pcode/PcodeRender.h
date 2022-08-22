#pragma once
#include "PcodeInstruction.h"

namespace sda::pcode
{
    class Render
    {
        const RegisterRepository* m_regRepo;
        size_t m_commentingCounter = 0;
    public:
        Render(const RegisterRepository* regRepo);

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
        StreamRender(std::ostream& output, const RegisterRepository* regRepo);

    protected:
        void renderTokenImpl(const std::string& text, Token token) const override;
    };
};