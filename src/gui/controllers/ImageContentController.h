#pragma once
#include "utilities/Helper.h"
#include <Zydis/Zydis.h>


namespace GUI
{
	class AbstractSectionController
	{
	public:
		CE::ImageDecorator* m_imageDec;
		const CE::ImageSection* m_imageSection;
		// mapping rows to memory offsets
		std::vector<uint64_t> m_offsets;

		AbstractSectionController(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
			: m_imageDec(imageDec), m_imageSection(imageSection)
		{}

		int8_t* getImageData() {
			return m_imageDec->getImage()->getData();
		}

		uint64_t getNextOffset(int row) {
			if (row == m_offsets.size() - 1)
				return m_imageSection->getMaxOffset();
			return m_offsets[row + 1];
		}

		int offsetToRow(uint64_t offset) const {
			using namespace Helper::Algorithm;
			size_t index = -1;
			BinarySearch(m_offsets, offset, index);
			return static_cast<int>(index);
		}

		// check if items(its bytes) were changed and then update {m_offsets}
		void checkItemLengthChanged(int row, int itemLength) {
			auto offset = m_offsets[row];
			auto nextOffset = getNextOffset(row);
			auto prevInstrLength = nextOffset - offset;
			if (itemLength < prevInstrLength) {
				const auto addByteOffsetsCount = prevInstrLength - itemLength;
				for (int i = 0; i < addByteOffsetsCount; i++)
					m_offsets.insert(m_offsets.begin() + row + 1, nextOffset - i);
			}
			else if (itemLength > prevInstrLength) {
				m_offsets.erase(m_offsets.begin() + row + 1);
				checkItemLengthChanged(row, itemLength);
			}
		}

		virtual void fillOffsets() = 0;
	};

	class DataSectionController : public AbstractSectionController
	{
	public:
		DataSectionController(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
			: AbstractSectionController(imageDec, imageSection)
		{
			fillOffsets();
		}

		CE::Symbol::AbstractSymbol* getSymbol(uint64_t offset) {
			auto symbol = m_imageDec->getGlobalSymbolTable()->getSymbolAt(offset).second;
			if (!symbol) {
				return m_imageDec->getImageManager()->getProject()->getSymbolManager()->getDefGlobalVarSymbol();
			}
			return symbol;
		}
	private:
		void fillOffsets() {
			m_offsets.clear();
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

		CodeSectionController(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
			: AbstractSectionController(imageDec, imageSection)
		{}

	protected:
		void addJump(uint64_t offset, uint64_t targetOffset) {
			Jmp jmp;
			jmp.m_startOffset = offset;
			jmp.m_endOffset = targetOffset;
			jmp.m_level = 0;
			m_jmps.push_back(jmp);

			if (m_offsetToJmp.find(offset) == m_offsetToJmp.end())
				m_offsetToJmp[offset] = std::list<Jmp*>();
			if (m_offsetToJmp.find(targetOffset) == m_offsetToJmp.end())
				m_offsetToJmp[targetOffset] = std::list<Jmp*>();
			auto pJmp = &*m_jmps.rbegin();
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

			std::function getJmp([&](std::map<uint64_t, std::list<Jmp*>>::iterator it)
				{
					auto& jmps = it->second;
					auto jmp = *jmps.begin();
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

		bool decodeZydisInstruction(uint64_t offset, ZydisDecodedInstruction* instruction) {
			if (ZYAN_FAILED(ZydisDecoderDecodeBuffer(&m_decoder, getImageData() + offset, 0x100, instruction))) {
				return false;
			}
			return true;
		}
	private:
		void fillOffsets() {
			m_offsets.clear();
			auto data = getImageData();
			auto offset = static_cast<int64_t>(m_imageSection->getMinOffset());
			ZydisDecodedInstruction instruction;
			while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&m_decoder, data + offset, m_imageSection->getMaxOffset() - offset,
				&instruction)))
			{
				if (instruction.meta.category == ZYDIS_CATEGORY_COND_BR || instruction.meta.category == ZYDIS_CATEGORY_UNCOND_BR) {
					const auto& operand = instruction.operands[0];
					if (operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
						if (operand.imm.is_relative) {
							auto targetOffset = offset + instruction.length + operand.imm.value.s;
							if (std::abs(offset - targetOffset) < 0x300) {
								addJump(offset, targetOffset);
							}
						}
					}
				}
				m_offsets.push_back(offset);
				offset += instruction.length;
			}

			setupJmpLevels();
		}
	};
};