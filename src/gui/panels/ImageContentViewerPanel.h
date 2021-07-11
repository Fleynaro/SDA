#pragma once
#include "ImageDecorator.h"
#include "imgui_internal.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/List.h"
#include "imgui_wrapper/controls/Text.h"
#include "utilities/Helper.h"
#include <Zydis/Zydis.h>

namespace GUI
{
	class ImageSectionListModel : public IListModel<const CE::ImageSection*>
	{
		class Iterator : public IListModel<const CE::ImageSection*>::Iterator
		{
			 std::list<CE::ImageSection>::const_iterator m_it;
		protected:
			ImageSectionListModel* m_listModel;
		public:
			Iterator(ImageSectionListModel* listModel)
				: m_listModel(listModel), m_it(listModel->m_image->getImageSections().begin())
			{}

			void getNextItem(std::string* text, const CE::ImageSection** data) override
			{
				*text = m_it->m_name;
				*data = &(*m_it);
				++m_it;
			}

			bool hasNextItem() override
			{
				return m_it != m_listModel->m_image->getImageSections().end();
			}
		};

		CE::AbstractImage* m_image;
	public:
		ImageSectionListModel(CE::AbstractImage* image)
			: m_image(image)
		{}

		void newIterator(const IteratorCallback& callback) override
		{
			Iterator iterator(this);
			callback(&iterator);
		}
	};
	
	class ImageContentViewerPanel : public AbstractPanel
	{
		inline const static ImGuiTableFlags TableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

		class AbstractSegmentViewer : public Control
		{
		protected:
			CE::ImageDecorator* m_imageDec;
			const CE::ImageSection* m_imageSection;
			std::vector<uint64_t> m_offsets;

			AbstractSegmentViewer(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
				: m_imageDec(imageDec), m_imageSection(imageSection)
			{}

			void renderAddressColumn(uint64_t offset) {
				using namespace Helper::String;
				ImGui::TableNextColumn();
				auto offsetStr = NumberToHex(offset + (static_cast<uint64_t>(1) << 63)).substr(1);
				Text::Text("base + " + offsetStr).show();
			}

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
		};

		class DataSegmentViewer : public AbstractSegmentViewer
		{
		public:
			DataSegmentViewer(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
				: AbstractSegmentViewer(imageDec, imageSection)
			{
				fillOffsets();
			}

		private:
			void renderControl() override {
				if (ImGui::BeginTable("content_table", 3, TableFlags))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Data type", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);
					ImGui::TableHeadersRow();

					ImGuiListClipper clipper;
					clipper.Begin(static_cast<int>(m_offsets.size()));
					while (clipper.Step())
					{
						for (auto row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
							ImGui::TableNextRow();
							auto offset = m_offsets[row];
							renderAddressColumn(offset);
							auto symbol = getSymbol(offset);
							checkItemLengthChanged(row, symbol->getSize());
							renderValueColumns(symbol, offset);
						}
					}
					ImGui::EndTable();
				}
			}

			CE::Symbol::AbstractSymbol* getSymbol(uint64_t offset) {
				auto symbol = m_imageDec->getGlobalSymbolTable()->getSymbolAt(offset).second;
				if (!symbol) {
					return m_imageDec->getImageManager()->getProject()->getSymbolManager()->getDefGlobalVarSymbol();
				}
				return symbol;
			}

			void renderValueColumns(CE::Symbol::AbstractSymbol* symbol, uint64_t offset) {
				using namespace Helper::String;

				auto pValue = reinterpret_cast<uint64_t*>(&getImageData()[offset]);
				ImGui::TableNextColumn();
				Text::Text(symbol->getDataType()->getDisplayName()).show();
				ImGui::TableNextColumn();
				Text::Text(symbol->getDataType()->getViewValue(*pValue)).show();
			}

			void fillOffsets() {
				m_offsets.clear();
				auto offset = m_imageSection->getMinOffset();
				while(offset < m_imageSection->getMaxOffset()) {
					m_offsets.push_back(offset);
					auto symbol = m_imageDec->getGlobalSymbolTable()->getSymbolAt(offset).second;
					if(symbol) {
						offset += symbol->getSize();
					} else {
						offset++;
					}
				}
			}
		};
		
		class CodeSegmentViewerX86 : public AbstractSegmentViewer
		{
			// todo: move to other class (bridge pattern), release Debugger viewer
			ZydisDecoder m_decoder;
			ZydisFormatter m_formatter;

			struct Jmp
			{
				int m_level;
				uint64_t m_startOffset;
				uint64_t m_endOffset;
			};
			std::list<Jmp> m_jmps;
			std::map<uint64_t, std::list<Jmp*>> m_offsetToJmp;
			std::set<Jmp*> m_shownJmps;
		public:
			CodeSegmentViewerX86(CE::ImageDecorator* imageDec, const CE::ImageSection* imageSection)
				: AbstractSegmentViewer(imageDec, imageSection)
			{
				ZydisDecoderInit(&m_decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
				ZydisFormatterInit(&m_formatter, ZYDIS_FORMATTER_STYLE_INTEL);
				fillOffsets();
			}

		private:
			void renderControl() override {
				m_shownJmps.clear();
				if (ImGui::BeginTable("content_table", 3, TableFlags))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Command", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Operands", ImGuiTableColumnFlags_None);
					ImGui::TableHeadersRow();

					ImGuiListClipper clipper;
					clipper.Begin(static_cast<int>(m_offsets.size()));
					while (clipper.Step())
					{
						for (auto row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
							ImGui::TableNextRow();
							auto offset = m_offsets[row];
							renderAddressColumn(offset);

							ZydisDecodedInstruction instruction;
							auto success = decodeZydisInstruction(offset, &instruction);
							if (success) {
								// todo: change also m_offsetToJmp when image bytes will be changed
								checkItemLengthChanged(row, instruction.length);
								success = renderInstructionColumns(row, &instruction, clipper);
							}
							
							if (!success) {
								ImGui::TableNextColumn();
								Text::Text("Error").show();
								ImGui::TableNextColumn();
								Text::Text("").show();
							}
							
						}
					}
					ImGui::EndTable();
				}
			}

			bool decodeZydisInstruction(uint64_t offset, ZydisDecodedInstruction* instruction) {
				if (ZYAN_FAILED(ZydisDecoderDecodeBuffer(&m_decoder, getImageData() + offset, 0x100, instruction))) {
					return false;
				}
				return true;
			}

			bool renderInstructionColumns(int row, ZydisDecodedInstruction* instruction, const ImGuiListClipper& clipper) {
				char buffer[256];
				const ZydisFormatterToken* token;
				if (ZYAN_FAILED(ZydisFormatterTokenizeInstruction(&m_formatter, instruction, &buffer[0],
					sizeof(buffer), ZYDIS_RUNTIME_ADDRESS_NONE, &token))) {
					return false;
				}
				
				ZydisTokenType token_type;
				ZyanConstCharPointer token_value = nullptr;
				std::string command;
				std::string operands;
				do
				{
					ZydisFormatterTokenGetValue(token, &token_type, &token_value);
					if(token_type == ZYDIS_TOKEN_MNEMONIC) {
						command = token_value;
					} else {
						// todo: colors, tooltip
						operands += token_value;
					}
				} while (ZYAN_SUCCESS(ZydisFormatterTokenNext(&token)));
				
				ImGui::TableNextColumn();
				Text::Text(command).show();

				// render jump arrow lines
				if (const auto it = m_offsetToJmp.find(m_offsets[row]); it != m_offsetToJmp.end()) {
					auto pJmps = it->second;
					for(auto pJmp : pJmps) {
						if (m_shownJmps.find(pJmp) == m_shownJmps.end()) {
							drawJmpLine(row, pJmp, clipper);
							m_shownJmps.insert(pJmp);
						}
					}
				}
				
				ImGui::TableNextColumn();
				Text::Text(operands).show();
				return true;
			}

			void drawJmpLine(int row, Jmp* pJmp, const ImGuiListClipper& clipper) {
				const float JmpLineLeftOffset = 10.0f;
				const float JmpLineGap = 8.0f;
				
				const bool isStart = m_offsets[row] == pJmp->m_startOffset;
				const auto targetRow = offsetToRow(isStart ? pJmp->m_endOffset : pJmp->m_startOffset);

				auto lineColor = ImGui::GetColorU32(ToImGuiColor(-1));
				const ImVec2 point1 = { ImGui::GetItemRectMin().x - 2.0f, (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) / 2.0f };
				const ImVec2 point2 = ImVec2(point1.x - pJmp->m_level * JmpLineGap - JmpLineLeftOffset, point1.y);
				const ImVec2 point3 = ImVec2(point2.x, point2.y + clipper.ItemsHeight * (targetRow - row));
				const ImVec2 point4 = ImVec2(point1.x, point3.y);

				// click event of the jump arrow
				ImGuiContext& g = *GImGui;
				if (ImGui::IsMousePosValid(&g.IO.MousePos)) {
					auto rect = ImRect(point2, { point3.x + JmpLineGap, point3.y });
					if(rect.Min.x > rect.Max.x || rect.Min.y > rect.Max.y)
						rect = ImRect(point3, { point2.x + JmpLineGap, point2.y });
					if (rect.Contains(g.IO.MousePos)) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						lineColor = ImGui::GetColorU32(ToImGuiColor(0x00bfffFF));
						if(ImGui::IsMouseClicked(0)) {
							const auto offsetToCenter = clipper.ItemsHeight * (clipper.DisplayEnd - clipper.DisplayStart) / 2.0f;
							ImGui::SetScrollY(targetRow * clipper.ItemsHeight - offsetToCenter);
						}
					}
				}


				// render the jump arrow
				ImGuiWindow* window = ImGui::GetCurrentWindow();
				window->DrawList->AddLine(point1, point2, lineColor);
				window->DrawList->AddLine(point2, point3, lineColor);
				window->DrawList->AddLine(point3, point4, lineColor);
				auto arrowPos = isStart ? point4 : point1;
				arrowPos.x -= 7.0f;
				arrowPos.y -= 3.0f;
				ImGui::RenderArrow(window->DrawList, arrowPos, lineColor, ImGuiDir_Right, 0.7f);
			}

			void fillOffsets() {
				m_offsets.clear();
				auto data = getImageData();
				auto offset = static_cast<int64_t>(m_imageSection->getMinOffset());
				ZydisDecodedInstruction instruction;
				while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&m_decoder, data + offset, m_imageSection->getMaxOffset() - offset,
					&instruction)))
				{
					if(instruction.meta.category == ZYDIS_CATEGORY_COND_BR || instruction.meta.category == ZYDIS_CATEGORY_UNCOND_BR) {
						const auto& operand = instruction.operands[0];
						if(operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
							if(operand.imm.is_relative) {
								auto targetOffset = offset + instruction.length + operand.imm.value.s;
								if (std::abs(offset - targetOffset) < 0x300) {
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
							}
						}
					}
					m_offsets.push_back(offset);
					offset += instruction.length;
				}

				setupJmpLevels();
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
		
		CE::ImageDecorator* m_imageDec;
		AbstractSegmentViewer* m_imageSectionViewer = nullptr; //todo: use map
		ImageSectionListModel m_imageSectionListModel;
		MenuListView<const CE::ImageSection*> m_imageSectionMenuListView;
	public:
		ImageContentViewerPanel(CE::ImageDecorator* imageDec)
			: AbstractPanel("Image content viewer"), m_imageDec(imageDec), m_imageSectionListModel(imageDec->getImage())
		{
			m_imageSectionMenuListView = MenuListView(&m_imageSectionListModel);
			m_imageSectionMenuListView.handler([&](const CE::ImageSection* imageSection)
				{
					selectImageSection(imageSection);
				});

			// select the code segment by default
			for(const auto& section : imageDec->getImage()->getImageSections()) {
				if(section.m_type == CE::ImageSection::CODE_SEGMENT) {
					selectImageSection(&section);
					break;
				}
			}
		}

		~ImageContentViewerPanel() override {
			delete m_imageSectionViewer;
		}

	private:
		void renderPanel() override {
			m_imageSectionViewer->show();
		}

		void renderMenuBar() override {
			if (ImGui::BeginMenu("Image section"))
			{
				m_imageSectionMenuListView.show();
				ImGui::EndMenu();
			}
		}

		void selectImageSection(const CE::ImageSection* imageSection) {
			delete m_imageSectionViewer;
			if (imageSection->m_type == CE::ImageSection::CODE_SEGMENT) {
				m_imageSectionViewer = new CodeSegmentViewerX86(m_imageDec, imageSection);
			}
			else {
				m_imageSectionViewer = new DataSegmentViewer(m_imageDec, imageSection);
			}
			m_imageSectionMenuListView.m_selectedItem = imageSection;
		}
	};
};