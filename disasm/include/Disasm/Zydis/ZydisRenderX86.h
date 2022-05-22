#pragma once
#include "Core/Pcode/PcodeVarnodes.h"

namespace sda::disasm
{
    class ZydisRegisterVarnodeRender : public pcode::RegisterVarnode::Render
    {
    public:
        std::string getRegisterName(const pcode::RegisterVarnode* varnode) const override;
    };
};
