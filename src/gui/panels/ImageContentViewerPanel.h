#pragma once
#include "ImageDecorator.h"
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
					clipper.Begin(m_offsets.size());
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
			ZydisDecoder m_decoder;
			ZydisFormatter m_formatter;
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
				if (ImGui::BeginTable("content_table", 3, TableFlags))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Command", ImGuiTableColumnFlags_None);
					ImGui::TableSetupColumn("Operands", ImGuiTableColumnFlags_None);
					ImGui::TableHeadersRow();

					ImGuiListClipper clipper;
					clipper.Begin(m_offsets.size());
					while (clipper.Step())
					{
						for (auto row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
							ImGui::TableNextRow();
							auto offset = m_offsets[row];
							renderAddressColumn(offset);

							ZydisDecodedInstruction instruction;
							auto success = decodeZydisInstruction(offset, &instruction);
							if (success) {
								checkItemLengthChanged(row, instruction.length);
								success = renderInstructionColumns(&instruction);
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

			bool renderInstructionColumns(ZydisDecodedInstruction* instruction) {
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
						// todo: colors
						operands += token_value;
					}
				} while (ZYAN_SUCCESS(ZydisFormatterTokenNext(&token)));
				
				ImGui::TableNextColumn();
				Text::Text(command).show();
				ImGui::TableNextColumn();
				Text::Text(operands).show();
			}

			void fillOffsets() {
				m_offsets.clear();
				auto data = getImageData();
				auto offset = m_imageSection->getMinOffset();
				ZydisDecodedInstruction instruction;
				while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&m_decoder, data + offset, m_imageSection->getMaxOffset() - offset,
					&instruction)))
				{
					m_offsets.push_back(offset);
					offset += instruction.length;
				}
			}
		};
		
		CE::ImageDecorator* m_imageDec;
		AbstractSegmentViewer* m_imageSectionViewer = nullptr;
		ImageSectionListModel m_imageSectionListModel;
		MenuListView<const CE::ImageSection*> imageSectionMenuListView;
	public:
		ImageContentViewerPanel(CE::ImageDecorator* imageDec)
			: AbstractPanel("Image content viewer"), m_imageDec(imageDec), m_imageSectionListModel(imageDec->getImage())
		{
			imageSectionMenuListView = MenuListView(&m_imageSectionListModel);
			imageSectionMenuListView.handler([&](const CE::ImageSection* imageSection)
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
				imageSectionMenuListView.show();
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
			imageSectionMenuListView.m_selectedItem = imageSection;
		}
	};
};