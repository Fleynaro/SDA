#include "DecompilerDemoPanel.h"
#include "ProjectManager.h"
#include <Project.h>
#include <asmtk/asmtk.h>
#include "panels/FunctionManagerPanel.h"
#include "panels/ImageManagerPanel.h"
#include "managers/Managers.h"
#include "panels/ImageContentViewerPanel.h"

using namespace CE;
using namespace asmjit;
using namespace asmtk;

std::string dumpCode(const uint8_t* buf, size_t size) {
    enum { kCharsPerLine = 39 };
    char hex[kCharsPerLine * 2 + 1];

    size_t i = 0;
    std::string result;
    while (i < size) {
        size_t j = 0;
        size_t end = size - i < kCharsPerLine ? size - i : size_t(kCharsPerLine);

        end += i;
        while (i < end) {
            uint8_t b0 = buf[i] >> 4;
            uint8_t b1 = buf[i] & 15;

            hex[j++] = b0 < 10 ? '0' + b0 : 'A' + b0 - 10;
            hex[j++] = b1 < 10 ? '0' + b1 : 'A' + b1 - 10;
            hex[j++] = ' ';
            i++;
        }

        hex[j] = '\0';
        result += hex;
    }

    result.pop_back();
    return result;
}

GUI::DecompilerDemoPanel::DecompilerDemoPanel()
	: AbstractPanel("Decompiler")
{
	// Controls
	m_asmCodeEditor = new Widget::CodeEditor("assembler code", ImVec2(200.0f, 300.0f));
	m_asmCodeEditor->getEditor().SetLanguageDefinition(TextEditor::LanguageDefinition::C());
	m_asmCodeEditor->getEditor().SetText(
		"mov eax, 0x10\n"
		"mov ebx, 0x20\n"
		"add eax, ebx\n"
		"mov [rsp], eax"
	);

	m_decCodeEditor = new Widget::CodeEditor("decompiled code", ImVec2(200.0f, 400.0f));
	m_decCodeEditor->getEditor().SetLanguageDefinition(TextEditor::LanguageDefinition::C());
	m_decCodeEditor->getEditor().SetReadOnly(true);

	m_asmParsingErrorText.setColor(ColorRGBA(0xFF0000FF));
	m_asmParsingErrorText.setDisplay(false);
	m_bytesParsingErrorText.setColor(ColorRGBA(0xFF0000FF));
	m_bytesParsingErrorText.setDisplay(false);
	m_decInfoText.setColor(ColorRGBA(0xFF0000FF));
	m_decInfoText.setDisplay(false);
	m_bytes_input = Input::TextInput();
	m_deassembly_btn = Button::StdButton("deassembly");
	m_assembly_btn = Button::StdButton("assembly");
	m_decompile_btn = Button::StdButton("decompile");

	m_tabBar = TabBar();

	// test models
	m_testListModel.addItem("item 1", 1);
	m_testListModel.addItem("item 2", 2);
	m_testListModel.addItem("item 3", 3);

	{
		auto rootNode = m_testTreeModel.getRootNode();
		rootNode->m_childNodes.push_back(m_testTreeModel.createNode("node 1", 1));
		{
			auto node2 = m_testTreeModel.createNode("node 2", 2);
			node2->m_childNodes.push_back(m_testTreeModel.createNode("node 21", 21));
			node2->m_childNodes.push_back(m_testTreeModel.createNode("node 22", 22));
			{
				auto node23 = m_testTreeModel.createNode("node 23", 23);
				node23->m_childNodes.push_back(m_testTreeModel.createNode("node 231", 231));
				node2->m_childNodes.push_back(node23);
			}
			rootNode->m_childNodes.push_back(node2);
		}
		rootNode->m_childNodes.push_back(m_testTreeModel.createNode("node 3", 3));
	}

	initProgram();
}

GUI::StdWindow* GUI::DecompilerDemoPanel::GetStdWindow()
{
	class Window : public StdWindow
	{
	public:
		friend class DecompilerDemoPanel;
		Window()
			: StdWindow(new DecompilerDemoPanel)
		{
			// Window params
			setFlags(
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
			setFullscreen(true);
		}

	private:
		void renderControl() override
		{
			auto panel = dynamic_cast<DecompilerDemoPanel*>(m_panel);
			panel->m_asmCodeEditor->getSize() = getSize();
			panel->m_asmCodeEditor->getSize().y *= 0.6f;
			panel->m_decCodeEditor->getSize() = getSize();
			StdWindow::renderControl();
		}
	};
	return new Window;
}

void GUI::DecompilerDemoPanel::renderPanel()
{
	m_tabBar.handler({
		TabItem("disassembler", [&]()
		{
			{
				Text::Text("Here write your assembler code:").show();
				m_asmCodeEditor->show();
				m_asmParsingErrorText.show();

				NewLine();
				if (m_deassembly_btn.present())
				{
					m_asmParsingErrorText.setDisplay(false);

					auto textCode = m_asmCodeEditor->getEditor().GetText();
					if (!textCode.empty())
					{
						deassembly(textCode);
					}
				}

				SameLine();
				if (m_assembly_btn.present())
				{
					m_bytesParsingErrorText.setDisplay(false);
					if (!m_bytes_input.getInputText().empty())
					{
						assembly(m_bytes_input.getInputText());
					}
				}
			}

			{
				NewLine();
				Separator();
				Text::Text("The machine code is presented as bytes in hexadecimal format:").show();
				m_bytes_input.show();
				m_bytesParsingErrorText.show();
				NewLine();
				if (m_decompile_btn.present())
				{
					m_bytesParsingErrorText.setDisplay(false);

					if (!m_bytes_input.getInputText().empty())
					{
						m_tabBar.selectTabItem("decompiler");
						decompile(m_bytes_input.getInputText());
					}
					else
					{
						m_bytesParsingErrorText.setDisplay(true);
						m_bytesParsingErrorText.setText("Please, enter hex bytes (format: 66 89 51 02)");
					}
				}
			}

			if (Button::StdButton("open item manager").present())
			{
				m_imageViewerWindow = new StdWindow(new ImageManagerPanel(m_project->getImageManager()));
			}
			if(m_popupBuiltinWindow)
				m_popupBuiltinWindow->placeAfterItem();

			if (Button::StdButton("open image content viewer").present())
			{
				auto panel = new ImageContentViewerPanel(m_project->getImageManager()->findImageByName("testRealImage"));
				m_imageContentViewerWindow = panel->createStdWindow();
			}

			if (Button::StdButton("open builtin popup").present()) {
				auto panel = new StdPanel();
				panel->handler([&]()
					{
						Text::Text("opened!").show();
					});
				m_popupBuiltinWindow = new PopupBuiltinWindow(panel);
				m_popupBuiltinWindow->open();
			}

			if (Button::StdButton("open popup").present()) {
				auto panel = new StdPanel("Popup");
				panel->handler([&]()
					{
						Text::Text("opened!").show();
					});
				m_popupModalWin = new PopupModalWindow(panel);
				m_popupModalWin->open();
			}
		}),

		TabItem("decompiler", [&]()
		{
			NewLine();
			Separator();
			Text::Text("Here the decompiled code is presented:").show();
			m_decCodeEditor->show();
			m_decInfoText.show();
		}),

		TabItem("extra", [&]()
		{
			StdListView listView(&m_testListModel);
			listView.handler([&](int value)
				{
					m_testListModel.addItem("added item", 10);
				});
			listView.show();

			NewLine();
			Separator();
			NewLine();

			TableListView tableListView(&m_testListModel);
			tableListView.show();

			NewLine();
			Separator();
			NewLine();

			StdTreeView treeView(&m_testTreeModel);
			treeView.handler([&](int value)
				{
					m_testListModel.addItem("added item", value);
				});
			treeView.show();
		})
	});
	m_tabBar.show();

	Show(m_functionManagerWindow);
	Show(m_imageViewerWindow);
	Show(m_imageContentViewerWindow);
	Show(m_popupModalWin);
	Show(m_popupBuiltinWindow);
	/*if(m_functionManagerWindow)
		dynamic_cast<FunctionManagerPanel*>(m_functionManagerWindow->getPanel())->m_filterControl.m_imageSelector.m_popupBuiltinWindow->show();*/
	ImGui::ShowDemoWindow();
}

// decompiler
#include <decompiler/DecMisc.h>
#include <images/VectorBufferImage.h>
#include <managers/Managers.h>

void GUI::DecompilerDemoPanel::initProgram() {
	auto prj_dir = m_program->getExecutableDirectory() / "demo2";
	fs::remove_all(prj_dir);
	
    m_program = new Program;
    m_project = m_program->getProjectManager()->createProject(prj_dir);

    m_project->initDataBase("database.db");
    m_project->initManagers();
    m_project->load();

	auto testAddrSpace = m_project->getAddrSpaceManager()->createAddressSpace("testAddrSpace1");
	auto testAddrSpace2 = m_project->getAddrSpaceManager()->createAddressSpace("testAddrSpace2");
	auto testImageDec = m_project->getImageManager()->createImage(testAddrSpace, ImageDecorator::IMAGE_PE, "testRealImage");
	{
		fs::copy_file(fs::path(TEST_DATA_PATH) / "images" / "img1.exe", testImageDec->getFile());
		testImageDec->load();
		Decompiler::WarningContainer warningContainer;
		Decompiler::RegisterFactoryX86 registerFactoryX86;
		Decompiler::PCode::DecoderX86 decoder(&registerFactoryX86, testImageDec->getInstrPool(), &warningContainer);

		Decompiler::ImageAnalyzer imageAnalyzer(testImageDec->getImage(), testImageDec->getPCodeGraph(), &decoder, &registerFactoryX86);
		// 0x20c80
		// 0x21fc0
		imageAnalyzer.start(0x20c80, true);
		
		for(auto funcGraph : testImageDec->getPCodeGraph()->getFunctionGraphList()) {
			const auto funcOffset = funcGraph.getStartBlock()->getMinOffset().getByteOffset();
			const auto suffix = std::to_string(funcOffset);
			const auto funcSignature = m_project->getTypeManager()->getFactory().createSignature("sig_" + suffix);
			m_project->getFunctionManager()->getFactory().createFunction(funcOffset, funcSignature, testImageDec, "func_" + suffix);
		}
	}
	m_project->getImageManager()->createImage(testAddrSpace, ImageDecorator::IMAGE_PE, "testImage12");
	m_project->getImageManager()->createImage(testAddrSpace2, ImageDecorator::IMAGE_PE, "testImage21");
	
	auto sig = m_project->getTypeManager()->getFactory().createSignature(DataType::CallingConvetion::FASTCALL, "sig");
	
	auto factory = m_project->getFunctionManager()->getFactory();
	factory.createFunction(0, sig, testImageDec, "createPlayer");
	factory.createFunction(10, sig, testImageDec, "createVehicle");
	factory.createFunction(20, sig, testImageDec, "setPlayerSpeed");
	m_project->getTransaction()->commit();
}

void GUI::DecompilerDemoPanel::deassembly(const std::string& textCode) {
    CodeHolder code;
    code.init(Environment(Environment::kArchX64));

    // Attach x86::Assembler `code`.
    x86::Assembler a(&code);

    // Create AsmParser that will emit to x86::Assembler.
    AsmParser p(&a);

    // Parse some assembly.
    Error err = p.parse(textCode.c_str());

    // Error handling (use asmjit::ErrorHandler for more robust error handling).
    if (err) {
        m_asmParsingErrorText.setDisplay(true);
        m_asmParsingErrorText.setText(std::string("Errors:\n") + DebugUtils::errorAsString(err));
        return;
    }

    // Now you can print the code, which is stored in the first section (.text).
    CodeBuffer& buffer = code.sectionById(0)->buffer();
    auto hexBytesStr = dumpCode(buffer.data(), buffer.size());
    m_bytes_input.setInputText(hexBytesStr);
}


int hexToDec(char c) {
    if (c <= '9')
        return c - '0';
    if (c >= 'a')
        return c - 'a' + 10;
    return c - 'A' + 10;
}

bool parseHexBytesStr(const std::string& hexBytesStr, std::vector<int8_t>& bytes) {
    int8_t b;
    for (int i = 0; i < hexBytesStr.length(); i ++) {
        if (i % 3 == 0)
            b = hexToDec(hexBytesStr[i]) << 4;
        else if (i % 3 == 1) {
            b |= hexToDec(hexBytesStr[i]);
            bytes.push_back(b);
        }
    }
    return true;
}

std::string getAsmListing(int8_t* data, ZyanUSize length) {
    std::string result;

    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);

    // Initialize formatter. Only required when you actually plan to do instruction
    // formatting ("disassembling"), like we do here
    ZydisFormatter formatter;
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    // Loop over the instructions in our buffer.
    // The runtime-address (instruction pointer) is chosen arbitrary here in order to better
    // visualize relative addressing
    ZyanUSize offset = 0;
    ZydisDecodedInstruction instruction;
    while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, data + offset, length - offset,
        &instruction)))
    {
        // Format & print the binary instruction structure to human readable format
        char buffer[256];
        ZydisFormatterFormatInstruction(&formatter, &instruction, buffer, sizeof(buffer),
            ZYDIS_RUNTIME_ADDRESS_NONE);

        result += std::string(buffer) + "\n";
        offset += instruction.length;
    }

    return result;
}

void GUI::DecompilerDemoPanel::assembly(const std::string& hexBytesStr)
{
    std::vector<int8_t> bytes;
    if (!parseHexBytesStr(hexBytesStr, bytes)) {
        m_bytesParsingErrorText.setDisplay(true);
        m_bytesParsingErrorText.setText("parsing error");
        return;
    }

    m_asmCodeEditor->getEditor().SetText(
        getAsmListing(bytes.data(), (ZyanUSize)bytes.size())
    );
}

void GUI::DecompilerDemoPanel::decompile(const std::string& hexBytesStr)
{
    using namespace CE::Decompiler;
    using namespace CE::Symbol;
    using namespace CE::DataType;

    std::vector<int8_t> bytes;
    if (!parseHexBytesStr(hexBytesStr, bytes)) {
        m_bytesParsingErrorText.setDisplay(true);
        m_bytesParsingErrorText.setText("parsing error");
        return;
    }

    auto image = VectorBufferImage(bytes);
    auto imageGraph = new ImagePCodeGraph;

    RegisterFactoryX86 registerFactoryX86;

    WarningContainer warningContainer;
    InstructionPool instrPool;
    PCode::DecoderX86 decoder(&registerFactoryX86, &instrPool, &warningContainer);
    PCodeGraphReferenceSearch graphReferenceSearch(m_project, &registerFactoryX86, &image);

    ImageAnalyzer imageAnalyzer(&image, imageGraph, &decoder, &registerFactoryX86, &graphReferenceSearch);
    imageAnalyzer.start(0x0, true);
    if (warningContainer.hasAnything()) {
        m_decInfoText.setText(warningContainer.getAllMessages());
    }

    // continue...
}

/*
	TODO
	1) Менеджер проектов
	2) Создание/редактирование функций, типов, образов (в менеджерах)
	3) Обозреватель ассемблерного кода (таблица)
	4) Декомпиляция функций (создание генератора)
	5) 
 */