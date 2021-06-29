#include <Program.h>
#include <Project.h>
#include "DecompilerDemoWindow.h"
#include <asmtk/asmtk.h>

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

void GUI::DecompilerDemoWindow::initProgram() {
    m_program = new Program;
    m_project = m_program->getProjectManager()->createProject("demo");

    m_project->initDataBase("database.db");
    m_project->initManagers();
    m_project->load();
}

void GUI::DecompilerDemoWindow::deassembly(const std::string& textCode) {
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


// decompiler
#include <Decompiler/DecMisc.h>
#include <Module/Image/VectorBufferImage.h>
#include <Manager/Managers.h>

int hexToDec(char c) {
    if (c <= '9')
        return c - '0';
    if (c >= 'a')
        return c - 'a' + 10;
    return c - 'A' + 10;
}

bool parseHexBytesStr(const std::string& hexBytesStr, std::vector<byte>& bytes) {
    byte b;
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

std::string getAsmListing(uint8_t* data, ZyanUSize length) {
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

void GUI::DecompilerDemoWindow::assembly(const std::string& hexBytesStr)
{
    std::vector<byte> bytes;
    if (!parseHexBytesStr(hexBytesStr, bytes)) {
        m_bytesParsingErrorText.setDisplay(true);
        m_bytesParsingErrorText.setText("parsing error");
        return;
    }

    m_asmCodeEditor->getEditor().SetText(
        getAsmListing(bytes.data(), (ZyanUSize)bytes.size())
    );
}

void GUI::DecompilerDemoWindow::decompile(const std::string& hexBytesStr)
{
    using namespace CE::Decompiler;
    using namespace CE::Symbol;
    using namespace CE::DataType;

    std::vector<byte> bytes;
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
