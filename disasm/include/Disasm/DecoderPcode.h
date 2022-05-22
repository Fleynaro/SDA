#pragma once
#include <string>
#include <vector>
#include <list>
#include "Core/Pcode/PcodeInstruction.h"

namespace sda::disasm
{
    class DecoderPcode
    {
    public:
        virtual void decode(Offset offset, const std::vector<uint8_t>& data) = 0;

        virtual size_t getInstructionLength() const = 0;

        std::list<pcode::Instruction>&& getDecodedInstructions();

        // Callbacks for the decoder
        class Callbacks
        {
        public:
            // Called when some warning is emitted
            virtual void onWarningEmitted(const std::string& message) {}
        };

        // Set the callbacks for the decoder
        std::unique_ptr<Callbacks> setCallbacks(std::unique_ptr<Callbacks> callbacks);

        // Get the callbacks for the decoder
        Callbacks* getCallbacks() const;

    private:
        std::unique_ptr<Callbacks> m_callbacks;

    protected:
        std::list<pcode::Instruction> m_decodedInstructions;
    };
};