#include "Core/Image/ImageAnalyser.h"
#include "Core/Image/Image.h"

using namespace sda;
using namespace sda::windows;

void PEImageAnalyser::analyse(Image* image) {
    m_image = image;
    analyseHeaders();
    analyseSections();
}

void PEImageAnalyser::analyseHeaders() {
    std::vector<uint8_t> imgDosHeader(sizeof __IMAGE_DOS_HEADER);
	m_image->getReader()->readBytesAtOffset(0x0, imgDosHeader);
    m_imgDosHeader = std::make_unique<__IMAGE_DOS_HEADER>();
	*m_imgDosHeader = *reinterpret_cast<__IMAGE_DOS_HEADER*>(imgDosHeader.data());
	
	const auto e_magic = reinterpret_cast<char*>(&m_imgDosHeader->e_magic);
	if (std::string(e_magic, 2) != "MZ")
		throw std::runtime_error("Invalid DOS header");

	std::vector<uint8_t> imgNtHeaders(sizeof __IMAGE_NT_HEADERS);
    m_image->getReader()->readBytesAtOffset(m_imgDosHeader->e_lfanew, imgNtHeaders);
    m_imgNtHeaders = std::make_unique<__IMAGE_NT_HEADERS>();
	*m_imgNtHeaders = *reinterpret_cast<__IMAGE_NT_HEADERS*>(imgNtHeaders.data());

	const auto signature = reinterpret_cast<char*>(&m_imgNtHeaders->Signature);
	if (std::string(signature, 2) != "PE")
		throw std::runtime_error("Invalid NT header");

    m_baseAddress = m_imgNtHeaders->OptionalHeader.ImageBase;
    m_entryPointOffset = m_imgNtHeaders->OptionalHeader.AddressOfEntryPoint;
}

void PEImageAnalyser::analyseSections() {
    ImageSection headerSection;
	headerSection.m_name = "PE HEADER";
	headerSection.m_virtualSize = m_imgDosHeader->e_lfanew + sizeof(__IMAGE_NT_HEADERS);
	m_imageSections.push_back(headerSection);

	for (int i = 0; i < m_imgNtHeaders->FileHeader.NumberOfSections; i++) {
		std::vector<uint8_t> imgSecHeader(sizeof __IMAGE_SECTION_HEADER);
		m_image->getReader()->readBytesAtOffset(
            m_imgDosHeader->e_lfanew + sizeof(__IMAGE_NT_HEADERS) + sizeof(__IMAGE_SECTION_HEADER) * i, imgSecHeader);
		const auto pSeh = reinterpret_cast<__IMAGE_SECTION_HEADER*>(imgSecHeader.data());

		ImageSection section;
		section.m_name = reinterpret_cast<const char*>(pSeh->Name);
		if (section.m_name == ".text")
			section.m_type = ImageSection::CODE_SEGMENT;
		if (section.m_name == ".data" || section.m_name == ".rdata")
			section.m_type = ImageSection::DATA_SEGMENT;
		section.m_relVirtualAddress = pSeh->VirtualAddress;
		section.m_virtualSize = pSeh->Misc.VirtualSize;
		section.m_pointerToRawData = pSeh->PointerToRawData;
		m_imageSections.push_back(section);
	}
}

void PEImageAnalyser::serialize(boost::json::object& data) const {
    data["type"] = Name;
}