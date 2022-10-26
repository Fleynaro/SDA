#include "PcodeFixture.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

std::list<pcode::Instruction> PcodeFixture::parsePcode(const std::string& text) const {
    return pcode::Parser::Parse(text, context->getPlatform()->getRegisterRepository().get());
}

