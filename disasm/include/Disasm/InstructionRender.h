#pragma once
#include <list>
#include <vector>
#include <string>
#include <ostream>

namespace sda::disasm
{
    // This instruction is used for rendering
    struct Instruction {
        struct Token {
            enum Type
            {
                Mneumonic,
                Register,
                AddressAbs,
                AddressRel,
                Other
            } type;
            std::string text;
        };
        
        std::list<Token> m_tokens;
        int m_length;

        class Render {
        public:
            virtual void render(const Instruction* instruction) const;

        protected:
			virtual void renderToken(const std::string& text, Token::Type token) const = 0;
        };

        class StreamRender : public Render {
            std::ostream& m_output;
        public:
            StreamRender(std::ostream& output);

        protected:
            void renderToken(const std::string& text, Token::Type token) const override;
        };
    };

    // This decoder is used to decode instructions for rendering
    class DecoderRender
    {
    public:
        virtual void decode(const std::vector<uint8_t>& data) = 0;

        const Instruction* getDecodedInstruction() const;
        
    protected:
        Instruction m_decodedInstruction;
    };
};