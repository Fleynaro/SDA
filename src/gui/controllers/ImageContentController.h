#pragma once
#include "decompiler/PCode/DecPCodeInstructionPool.h"
#include "managers/ImageManager.h"
#include "managers/SymbolManager.h"
#include "utilities/Helper.h"
#include <Zydis/Zydis.h>


namespace GUI
{
	class AbstractSectionController
	{
	public:
		CE::ImageDecorator* m_imageDec;
		const CE::ImageSection* m_imageSection;

		AbstractSectionController(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
			: m_imageDec(imageDec), m_imageSection(imageSection)
		{}

		virtual void update() = 0;
	};

	struct AbstractRow
	{
		operator uint64_t() const {
			return *reinterpret_cast<const uint64_t*>(this);
		}
		
		bool operator==(const AbstractRow& other) const {
			return static_cast<uint64_t>(*this) == static_cast<uint64_t>(other);
		}

		bool operator<(const AbstractRow& other) const {
			return static_cast<uint64_t>(*this) < static_cast<uint64_t>(other);
		}
	};
	
	template<typename T = uint64_t>
	class AbstractSectionControllerWithRows : public AbstractSectionController
	{
	protected:
		std::vector<T> m_rows;
	public:
		using AbstractSectionController::AbstractSectionController;
		
		int getRowsCount() const {
			return static_cast<int>(m_rows.size());
		}

		T getRow(int idx) const {
			if (idx == getRowsCount())
				return m_imageSection->getMaxOffset();
			return m_rows[idx];
		}

		int getRowIdx(T row) const {
			using namespace Helper::Algorithm;
			int index = -1;
			BinarySearch<T>(m_rows, row, index);
			return index;
		}
	};

	class DataSectionController : public AbstractSectionControllerWithRows<uint64_t>
	{
	public:
		DataSectionController(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
			: AbstractSectionControllerWithRows<uint64_t>(imageDec, imageSection)
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
			m_rows.clear();
			fillOffsets();
		}
	private:
		void fillOffsets() {
			auto offset = m_imageSection->getMinOffset();
			while (offset < m_imageSection->getMaxOffset()) {
				m_rows.push_back(offset);
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

	struct CodeSectionRow : AbstractRow {
		union {
			struct {
				uint64_t m_isPCode : 1;
				uint64_t m_orderId : 8;
				uint64_t m_byteOffset : 55;
			};
			struct {
				uint64_t : 1;
				uint64_t m_fullOffset : 63;
			};
		};

		CodeSectionRow(CE::Offset offset, int orderId, bool isPCode)
			: m_byteOffset(offset), m_orderId(orderId), m_isPCode(isPCode)
		{}

		CodeSectionRow(CE::ComplexOffset offset, bool isPCode = false)
			: m_fullOffset(offset), m_isPCode(isPCode)
		{}

		CE::ComplexOffset getOffset() const {
			return CE::ComplexOffset(m_byteOffset, m_orderId);
		}
	};
	
	class CodeSectionController : public AbstractSectionControllerWithRows<CodeSectionRow>
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
			: AbstractSectionControllerWithRows<CodeSectionRow>(imageDec, imageSection)
		{}

		void update() override {
			m_rows.clear();
			m_jmps.clear();
			m_offsetToJmp.clear();
		}
	
	protected:
		void addOffset(CE::Offset offset) {
			m_rows.emplace_back(offset, 0, false);
			if (m_showPCode) {
				if (auto origInstr = m_imageDec->getInstrPool()->getOrigInstructionAt(offset)) {
					for (const auto& pair : origInstr->m_pcodeInstructions) {
						const auto pcodeInstr = &pair.second;
						m_rows.emplace_back(pcodeInstr->getOffset(), true);
					}
				}
			}
		}
		
		void addJump(CE::Offset offset, CE::Offset targetOffset) {
			offset <<= 8;
			targetOffset <<= 8;
			Jmp jmp;
			jmp.m_startOffset = offset;
			jmp.m_endOffset = targetOffset;
			jmp.m_level = 0;
			m_jmps.push_back(jmp);

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

		void update() override {
			CodeSectionController::update();
			fillOffsets();
		}
	
	private:
		void fillOffsets() {
			auto offset = m_imageSection->getMinOffset();
			ZydisDecodedInstruction instruction;
			while (decode(offset, &instruction))
			{
				if (instruction.meta.category == ZYDIS_CATEGORY_COND_BR || instruction.meta.category == ZYDIS_CATEGORY_UNCOND_BR) {
					const auto& operand = instruction.operands[0];
					if (operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
						if (operand.imm.is_relative) {
							const auto targetOffset = offset + instruction.length + operand.imm.value.s;
							if (std::abs(static_cast<int64_t>(offset) - static_cast<int64_t>(targetOffset)) < 0x300) {
								addJump(offset, targetOffset);
							}
						}
					}
				}
				addOffset(offset);
				offset += instruction.length;
			}

			setupJmpLevels();
		}

		bool decode(CE::Offset offset, ZydisDecodedInstruction* instruction) const {
			std::vector<uint8_t> buffer(100);
			m_imageDec->getImage()->read(offset, buffer);
			return ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&m_decoder, buffer.data(), buffer.size(), instruction));
		}
	};
};