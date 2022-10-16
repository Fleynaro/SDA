#pragma once
#include <string>
#include <vector>
#include <list>
#include "SDA/Core/Pcode/PcodeInstruction.h"

namespace sda
{
    class PcodeDecoder
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
        void setCallbacks(std::shared_ptr<Callbacks> callbacks);

        // Get the callbacks for the decoder
        std::shared_ptr<Callbacks> getCallbacks() const;

    private:
        std::shared_ptr<Callbacks> m_callbacks;

    protected:
        std::list<pcode::Instruction> m_decodedInstructions;
    };
};