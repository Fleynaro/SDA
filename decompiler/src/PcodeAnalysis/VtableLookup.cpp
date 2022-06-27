#include "Decompiler/PcodeAnalysis/VtableLookup.h"
#include "Core/Utils.h"

using namespace sda;
using namespace sda::decompiler;

VtableLookupCallbacks::VtableLookupCallbacks(Image* image, PcodeBlockBuilder* builder, std::unique_ptr<Callbacks> nextCallbacks)
    : m_image(image), PcodeBlockBuilder::StdCallbacks(builder, std::move(nextCallbacks))
{}

const std::list<VtableLookupCallbacks::VTable>& VtableLookupCallbacks::getVtables() const {
    return m_vtables;
}

void VtableLookupCallbacks::onInstructionPassed(const pcode::Instruction* instr, pcode::InstructionOffset nextOffset) {
    if (instr->getId() == pcode::InstructionId::COPY) {
        if (const auto constVarnode = std::dynamic_pointer_cast<pcode::ConstantVarnode>(instr->getInput0())) {
            Offset vtableRva = constVarnode->getValue();
            if (auto section = m_image->getImageSectionAt(vtableRva)) {
                if (section->m_type == ImageSection::DATA_SEGMENT) {
                    std::list<Offset> funcOffsets;
                    
                    // assuming it is a vtable (not global var), get the function offsets from it
                    auto offset = m_image->toImageFileOffset(vtableRva);
                    while (offset < m_image->getSize()) {
                        std::vector<uint8_t> buffer(sizeof Offset);
                        m_image->getRW()->readBytesAtOffset(offset, buffer);

                        const auto funcAddr = *reinterpret_cast<Offset*>(buffer.data());
                        if (funcAddr == 0x0) // all vtables ends with zero address
                            break;

                        const auto funcRva = m_image->toOffset(funcAddr);
                        auto section = m_image->getImageSectionAt(funcRva);
                        if (section && section->m_type == ImageSection::CODE_SEGMENT) {
                            funcOffsets.push_back(funcRva);
                            m_builder->addUnvisitedOffset(pcode::InstructionOffset(funcRva, 0));
                        }
                        offset += sizeof(Offset);
                    }

                    if (funcOffsets.empty())
                        m_vtables.push_back(VTable({vtableRva, funcOffsets}));
                }
            }
        }
    }

    if (m_nextCallbacks)
        m_nextCallbacks->onInstructionPassed(instr, nextOffset);
}