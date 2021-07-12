#pragma once
#include "controllers/ImageContentController.h"
#include "imgui_wrapper/controls/Control.h"

namespace GUI
{
	class AbstractInstructionViewer : public Control
	{
	public:
		class ITokenRender
		{
		public:
			virtual void renderMnemonic(const std::string& token) = 0;

			virtual void renderRegister(const std::string& token) = 0;

			virtual void renderAbsAddress(const std::string& token) = 0;

			virtual void renderRelAddress(const std::string& token) = 0;

			virtual void renderOther(const std::string& token) = 0;

			virtual void render() = 0;
		};

		class AbstractTokenRenderText : public ITokenRender
		{
		protected:
			std::string m_command;
			std::string m_operands;
		public:
			void renderMnemonic(const std::string& token) override {
				m_command = token;
			}

			void renderRegister(const std::string& token) override {
				m_operands += token;
			}

			void renderAbsAddress(const std::string& token) override {
				m_operands += token;
			}

			void renderRelAddress(const std::string& token) override {
				m_operands += token;
			}

			void renderOther(const std::string& token) override {
				m_operands += token;
			}
		};
	protected:
		uint64_t m_offset;
		ITokenRender* m_tokenRender = nullptr;

	public:
		void setOffset(uint64_t offset) {
			m_offset = offset;
		}

		void setTokenRender(ITokenRender* tokenRender) {
			m_tokenRender = tokenRender;
		}
	};

	class InstructionViewerX86 : public AbstractInstructionViewer
	{
		CodeSectionControllerX86* m_codeSectionController;
		ZydisFormatter m_formatter;
	public:
		InstructionViewerX86(CodeSectionControllerX86* codeSectionController)
			: m_codeSectionController(codeSectionController)
		{
			ZydisFormatterInit(&m_formatter, ZYDIS_FORMATTER_STYLE_INTEL);
		}

	private:
		void renderControl() override {
			ZydisDecodedInstruction instruction;
			m_codeSectionController->decodeZydisInstruction(m_offset, &instruction);

			char buffer[256];
			const ZydisFormatterToken* token;
			if (ZYAN_FAILED(ZydisFormatterTokenizeInstruction(&m_formatter, &instruction, &buffer[0],
				sizeof(buffer), ZYDIS_RUNTIME_ADDRESS_NONE, &token))) {
				m_tokenRender->renderMnemonic("error");
				m_tokenRender->render();
				return;
			}

			ZydisTokenType token_type;
			ZyanConstCharPointer token_value = nullptr;
			do
			{
				ZydisFormatterTokenGetValue(token, &token_type, &token_value);
				if (token_type == ZYDIS_TOKEN_MNEMONIC) {
					m_tokenRender->renderMnemonic(token_value);
				}
				else if (token_type == ZYDIS_TOKEN_REGISTER) {
					m_tokenRender->renderRegister(token_value);
				}
				else if (token_type == ZYDIS_TOKEN_ADDRESS_ABS) {
					m_tokenRender->renderAbsAddress(token_value);
				}
				else if (token_type == ZYDIS_TOKEN_ADDRESS_REL) {
					m_tokenRender->renderRelAddress(token_value);
				}
				else {
					m_tokenRender->renderOther(token_value);
				}
			} while (ZYAN_SUCCESS(ZydisFormatterTokenNext(&token)));
			m_tokenRender->render();
		}
	};
};