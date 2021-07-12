#pragma once
#include "decompiler/PCode/DecPCodeInstructionPool.h"
#include "utilities/Helper.h"
#include <Zydis/Zydis.h>


namespace GUI
{
	class AbstractSectionController
	{
	protected:
		// mapping rows to memory offsets
		std::vector<uint64_t> m_offsets;
	public:
		CE::ImageDecorator* m_imageDec;
		const CE::ImageSection* m_imageSection;

		AbstractSectionController(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
			: m_imageDec(imageDec), m_imageSection(imageSection)
		{}

		int8_t* getImageData() const {
			return m_imageDec->getImage()->getData();
		}

		int getOffsetsCount() const {
			return static_cast<int>(m_offsets.size());
		}
		
		uint64_t getOffset(int row) const {
			if (row == getOffsetsCount())
				return m_imageSection->getMaxOffset();
			return m_offsets[row];
		}

		int offsetToRow(uint64_t offset) const {
			using namespace Helper::Algorithm;
			size_t index = -1;
			BinarySearch(m_offsets, offset, index);
			return static_cast<int>(index);
		}

		virtual void update() = 0;
	};

	class DataSectionController : public AbstractSectionController
	{
	public:
		DataSectionController(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
			: AbstractSectionController(imageDec, imageSection)
		{
			fillOffsets();
		}

		CE::Symbol::AbstractSymbol* getSymbol(uint64_t offset) const {
			const auto symbol = m_imageDec->getGlobalSymbolTable()->getSymbolAt(offset).second;
			if (!symbol) {
				return m_imageDec->getImageManager()->getProject()->getSymbolManager()->getDefGlobalVarSymbol();
			}
			return symbol;
		}

		void update() override {
			m_offsets.clear();
			fillOffsets();
		}
	private:
		void fillOffsets() {
			auto offset = m_imageSection->getMinOffset();
			while (offset < m_imageSection->getMaxOffset()) {
				m_offsets.push_back(offset);
				auto symbol = m_imageDec->getGlobalSymbolTable()->getSymbolAt(offset).second;
				if (symbol) {
					offset += symbol->getSize();
				}
				else {
					offset++;
				}
			}
		}
	};

	class CodeSectionController : public AbstractSectionController
	{
	public:
		struct Jmp
		{
			int m_level;
			uint64_t m_startOffset;
			uint64_t m_endOffset;
		};
		std::list<Jmp> m_jmps;
		std::map<uint64_t, std::list<Jmp*>> m_offsetToJmp;
		bool m_showPCode = false;

		CodeSectionController(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
			: AbstractSectionController(imageDec, imageSection)
		{}

		void update() override {
			m_offsets.clear();
			m_jmps.clear();
			m_offsetToJmp.clear();
		}

		uint64_t getInstrOffset(int row) const {
			return getOffset(row) >> 1;
		}

		int instrOffsetToRow(uint64_t instrOffset) const {
			return offsetToRow(instrOffset << 1);
		}
	
	protected:
		void addOffset(uint64_t offset) {
			m_offsets.push_back(offset << (8 + 1));
			if (m_showPCode) {
				if (auto origInstr = m_imageDec->getInstrPool()->getOrigInstructionAt(offset)) {
					for (const auto& pair : origInstr->m_pcodeInstructions) {
						auto pcodeInstr = &pair.second;
						m_offsets.push_back((pcodeInstr->getOffset() << 1) | 1);
					}
				}
			}
		}
		
		void addJump(uint64_t offset, uint64_t targetOffset) {
			offset <<= 8;
			targetOffset <<= 8;
			Jmp jmp;
			jmp.m_startOffset = offset;
			jmp.m_endOffset = targetOffset;
			jmp.m_level = 0;
			m_jmps.push_back(jmp);

			if (m_offsetToJmp.find(offset) == m_offsetToJmp.end())
				m_offsetToJmp[offset] = std::list<Jmp*>();
			if (m_offsetToJmp.find(targetOffset) == m_offsetToJmp.end())
				m_offsetToJmp[targetOffset] = std::list<Jmp*>();
			const auto pJmp = &*m_jmps.rbegin();
			m_offsetToJmp[offset].push_back(pJmp);
			m_offsetToJmp[targetOffset].push_back(pJmp);
		}
		
		// calculate levels for all jump arrow lines
		void setupJmpLevels() {
			std::map<uint64_t, std::list<Jmp*>> offToJmps;
			for (auto& jmp : m_jmps) {
				// calculate lower bound of jump
				auto minOffset = std::min(jmp.m_startOffset, jmp.m_endOffset);
				if (offToJmps.find(minOffset) == offToJmps.end())
					offToJmps[minOffset] = std::list<Jmp*>();
				offToJmps[minOffset].push_back(&jmp);
			}

			const std::function getJmp([&](std::map<uint64_t, std::list<Jmp*>>::iterator it)
				{
					auto& jmps = it->second;
					const auto jmp = *jmps.begin();
					jmps.pop_front();
					if (jmps.empty())
						offToJmps.erase(it);
					return jmp;
				});

			// passing layer by layer and assigning layer level
			int layerLevel = 1;
			while (!offToJmps.empty()) {
				auto it = offToJmps.begin();
				auto jmp = getJmp(it);
				jmp->m_level = layerLevel;

				// finding all non-intersected jumps
				while (true) {
					// calculate upper bound of jump
					auto maxOffset = std::max(jmp->m_startOffset, jmp->m_endOffset);
					it = offToJmps.upper_bound(maxOffset);
					if (it == offToJmps.end())
						break;
					jmp = getJmp(it);
					jmp->m_level = layerLevel;
				}
				// go the next layer
				layerLevel++;
			}
		}
	};

	class CodeSectionControllerX86 : public CodeSectionController
	{
		ZydisDecoder m_decoder;
	public:
		CodeSectionControllerX86(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
			: CodeSectionController(imageDec, imageSection)
		{
			ZydisDecoderInit(&m_decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
			fillOffsets();
		}

		bool decodeZydisInstruction(uint64_t offset, ZydisDecodedInstruction* instruction) const {
			if (ZYAN_FAILED(ZydisDecoderDecodeBuffer(&m_decoder, getImageData() + m_imageSection->toImageOffset(offset), 0x100, instruction))) {
				return false;
			}
			return true;
		}

		void update() override {
			CodeSectionController::update();
			fillOffsets();
		}
	
	private:
		void fillOffsets() {
			const auto data = getImageData();
			auto offset = static_cast<int64_t>(m_imageSection->getMinOffset());
			auto size = m_imageSection->m_virtualSize;
			ZydisDecodedInstruction instruction;
			while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&m_decoder, data + m_imageSection->toImageOffset(offset), size, &instruction)))
			{
				if (instruction.meta.category == ZYDIS_CATEGORY_COND_BR || instruction.meta.category == ZYDIS_CATEGORY_UNCOND_BR) {
					const auto& operand = instruction.operands[0];
					if (operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
						if (operand.imm.is_relative) {
							const auto targetOffset = offset + instruction.length + operand.imm.value.s;
							if (std::abs(offset - targetOffset) < 0x300) {
								addJump(offset, targetOffset);
							}
						}
					}
				}
				addOffset(offset);
				offset += instruction.length;
				size -= instruction.length;
			}

			setupJmpLevels();
		}
	};
};