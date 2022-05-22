#include "Disasm/Zydis/ZydisRenderX86.h"
#include <Zydis/Zydis.h>

using namespace sda::disasm;

std::string GetFlagName(size_t flag) {
    std::string flagName = "flag";
    if (flag == ZYDIS_CPUFLAG_CF)
        flagName = "CF";
    else if (flag == ZYDIS_CPUFLAG_OF)
        flagName = "OF";
    else if (flag == ZYDIS_CPUFLAG_SF)
        flagName = "SF";
    else if (flag == ZYDIS_CPUFLAG_ZF)
        flagName = "ZF";
    else if (flag == ZYDIS_CPUFLAG_AF)
        flagName = "AF";
    else if (flag == ZYDIS_CPUFLAG_PF)
        flagName = "PF";
    return flagName;
}

std::string ZydisRegisterVarnodeRender::getRegisterName(const pcode::RegisterVarnode* varnode) const {
    if (varnode->getType() == pcode::RegisterVarnode::Flag)
        return GetFlagName(varnode->getId()) + ":1";

    auto size = varnode->getSize();
    auto maskStr = std::to_string(size);
    if (varnode->getType() == pcode::RegisterVarnode::Vector) {
        if (size == 4 || size == 8) {
            maskStr = std::string(size == 4 ? "D" : "Q");
            auto maskOffset = varnode->getMask().getOffset();
            maskStr += static_cast<char>('a' + static_cast<char>(maskOffset / (size * 8)));
        }
    }

    const auto regId = static_cast<ZydisRegister>(varnode->getId());
    return std::string(ZydisRegisterGetString(regId)) + ":" + maskStr;
}