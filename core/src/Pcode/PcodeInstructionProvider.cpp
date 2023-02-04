#include "SDA/Core/Pcode/PcodeInstructionProvider.h"
#include "SDA/Core/Image/Image.h"

using namespace sda::pcode;

ListInstructionProvider::ListInstructionProvider(const std::list<pcode::Instruction>& instructions)
{
    for (const auto& instr : instructions) {
        m_instructions[instr.getOffset()] = instr;
    }
}

void ListInstructionProvider::decode(Offset offset, std::list<pcode::Instruction>& instructions, size_t& origInstrLength) {
    const auto it = m_instructions.find(pcode::InstructionOffset(offset, 0));
    if (it == m_instructions.end())
        throw std::runtime_error("Pcode instruction not found");
    instructions.push_back(it->second);
    origInstrLength = 1;
}

bool ListInstructionProvider::isOffsetValid(Offset offset) {
    return m_instructions.find(pcode::InstructionOffset(offset, 0)) != m_instructions.end();
}

ImageInstructionProvider::ImageInstructionProvider(Image* image)
    : m_image(image)
{
    m_decoder = m_image->getContext()->getPlatform()->getPcodeDecoder();
}

void ImageInstructionProvider::decode(Offset offset, std::list<pcode::Instruction>& instructions, size_t& origInstrLength) {
    std::vector<uint8_t> data(100);
    m_image->getRW()->readBytesAtOffset(offset, data);
    m_decoder->decode(offset, data);
    instructions = m_decoder->getDecodedInstructions();
    origInstrLength = m_decoder->getInstructionLength();
}

bool ImageInstructionProvider::isOffsetValid(Offset offset) {
    if (offset >= m_image->getSize())
        return false;
    auto section = m_image->getImageSectionAt(offset);
    return section && section->m_type == ImageSection::CODE_SEGMENT;
}
