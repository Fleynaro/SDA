#include "Platform/X86/Platform.h"
#include "Platform/X86/RegisterRepository.h"
#include "Platform/X86/PcodeDecoder.h"
#include "Platform/X86/InstructionDecoder.h"
#include "Platform/X86/CallingConvention.h"

using namespace sda;
using namespace sda::platform;

PlatformX86::PlatformX86(bool is64Version)
    : m_is64Version(is64Version)
{
    m_regRepo = std::make_unique<RegisterRepositoryX86>();
    m_callingConventions = {
        std::make_shared<FastcallCallingConvention>()
    };
}

std::string PlatformX86::getName() const {
    return m_is64Version ? "x86-64" : "x86";
}

size_t PlatformX86::getPointerSize() const {
    return m_is64Version ? 8 : 4;
}

RegisterRepository* PlatformX86::getRegisterRepository() const {
    return m_regRepo.get();
}

const std::list<std::shared_ptr<CallingConvention>>& PlatformX86::getCallingConventions() const {
    return m_callingConventions;
}

std::shared_ptr<PcodeDecoder> PlatformX86::getPcodeDecoder() const {
    auto decoder = std::make_unique<ZydisDecoder>();
    ZydisDecoderInit(decoder.get(), ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
    return std::make_shared<PcodeDecoderX86>(std::move(decoder));
}

std::shared_ptr<InstructionDecoder> PlatformX86::getInstructionDecoder() const {
    auto decoder = std::make_unique<ZydisDecoder>();
    ZydisDecoderInit(decoder.get(), ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
    auto formatter = std::make_unique<ZydisFormatter>();
    ZydisFormatterInit(formatter.get(), ZYDIS_FORMATTER_STYLE_INTEL);
    return std::make_shared<InstructionDecoderX86>(std::move(decoder), std::move(formatter));
}