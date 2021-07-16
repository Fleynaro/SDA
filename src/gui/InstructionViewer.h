#pragma once
#include "controllers/ImageContentController.h"
#include "imgui_wrapper/controls/Control.h"

namespace GUI
{
	struct InstructionViewInfo
	{
		enum Type
		{
			TOKEN_MNEMONIC,
			TOKEN_REGISTER,
			TOKEN_ADDRESS_ABS,
			TOKEN_ADDRESS_REL,
			TOKEN_OTHER
		};
		
		struct Token
		{
			Type m_type;
			std::string m_text;
		};
		
		std::list<Token> m_tokens;
		int m_length;
	};

	class AbstractInstructionViewer : public Control
	{
		InstructionViewInfo* m_instrViewInfo;
	public:
		AbstractInstructionViewer(InstructionViewInfo* instrViewInfo)
			: m_instrViewInfo(instrViewInfo)
		{}

	protected:
		virtual void renderMnemonic() {
			const auto mnemonic = m_instrViewInfo->m_tokens.begin()->m_text;
			Text::ColoredText(mnemonic, 0xe6e4b3FF).show();
		}

		virtual void renderOperands() {
			std::string operands;
			for (auto it = std::next(m_instrViewInfo->m_tokens.begin()); it != m_instrViewInfo->m_tokens.end(); ++it) {
				ColorRGBA color = 0xebebebFF;
				if (it->m_type == InstructionViewInfo::TOKEN_REGISTER)
					color = 0xb3e6e4FF;
				else if (it->m_type == InstructionViewInfo::TOKEN_ADDRESS_REL || it->m_type == InstructionViewInfo::TOKEN_ADDRESS_ABS)
					color = 0xafcfa7FF;
				Text::ColoredText(it->m_text, color).show();
				SameLine(1.0f);
			}
		}

	private:
		void renderControl() override {
			renderMnemonic();
			renderOperands();
		}
	};

	class InstructionTableRowViewer : public AbstractInstructionViewer
	{
	public:
		using AbstractInstructionViewer::AbstractInstructionViewer;
	
	protected:
		void renderMnemonic() override {
			ImGui::TableNextColumn();
			AbstractInstructionViewer::renderMnemonic();
		}

		void renderOperands() override {
			ImGui::TableNextColumn();
			AbstractInstructionViewer::renderOperands();
		}
	};
	
	class AbstractInstructionViewDecoder
	{
	public:
		virtual bool decode(const void* addr, InstructionViewInfo* instrViewInfo) = 0;
	};

	class InstructionViewDecoderX86 : public AbstractInstructionViewDecoder
	{
		ZydisDecoder m_decoder;
		ZydisFormatter m_formatter;
	public:
		InstructionViewDecoderX86()
		{
			ZydisDecoderInit(&m_decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
			ZydisFormatterInit(&m_formatter, ZYDIS_FORMATTER_STYLE_INTEL);
		}

		bool decode(const void* addr, InstructionViewInfo* instrViewInfo) override {
			ZydisDecodedInstruction instruction;
			if (ZYAN_FAILED(ZydisDecoderDecodeBuffer(&m_decoder, addr, 0x100, &instruction))) {
				return false;
			}

			instrViewInfo->m_length = instruction.length;
			
			char buffer[256];
			const ZydisFormatterToken* token;
			if (ZYAN_FAILED(ZydisFormatterTokenizeInstruction(&m_formatter, &instruction, &buffer[0],
				sizeof(buffer), ZYDIS_RUNTIME_ADDRESS_NONE, &token))) {
				return false;
			}

			ZydisTokenType token_type;
			ZyanConstCharPointer token_value = nullptr;
			do
			{
				ZydisFormatterTokenGetValue(token, &token_type, &token_value);
				if (token_type == ZYDIS_TOKEN_MNEMONIC) {
					instrViewInfo->m_tokens.push_back({InstructionViewInfo::TOKEN_MNEMONIC, token_value });
				}
				else if (token_type == ZYDIS_TOKEN_REGISTER) {
					instrViewInfo->m_tokens.push_back({ InstructionViewInfo::TOKEN_REGISTER, token_value });
				}
				else if (token_type == ZYDIS_TOKEN_ADDRESS_ABS) {
					instrViewInfo->m_tokens.push_back({ InstructionViewInfo::TOKEN_ADDRESS_ABS, token_value });
				}
				else if (token_type == ZYDIS_TOKEN_ADDRESS_REL) {
					instrViewInfo->m_tokens.push_back({ InstructionViewInfo::TOKEN_ADDRESS_REL, token_value });
				}
				else {
					instrViewInfo->m_tokens.push_back({ InstructionViewInfo::TOKEN_OTHER, token_value });
				}
			} while (ZYAN_SUCCESS(ZydisFormatterTokenNext(&token)));
			return true;
		}
	};

	class PCodeInstructionRender : public IRender, CE::Decompiler::PCode::InstructionViewGenerator
	{
	public:
		using InstructionViewGenerator::InstructionViewGenerator;

		void render() override {
			generate();
		}
	private:
		void generateToken(const std::string& text, TokenType tokenType) override {
			ColorRGBA color = 0xebebebFF;
			if (tokenType == TOKEN_MNEMONIC)
				color = 0xe6e4b3FF;
			else if (tokenType == TOKEN_REGISTER)
				color = 0xb3e6e4FF;
			else if (tokenType == TOKEN_VARIABLE)
				color = 0xaac5f2FF;
			else if (tokenType == TOKEN_NUMBER)
				color = 0xc7c7c7FF;
			Text::ColoredText(text, color).show();
			SameLine(1.0f);
		}
	};
};