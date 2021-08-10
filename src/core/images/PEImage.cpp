#include "PEImage.h"

CE::PEImage::PEImage(IReader* reader)
	: AbstractImage(reader)
{}

void CE::PEImage::analyze() {
	parse();
	loadSections();
}

int CE::PEImage::getOffsetOfEntryPoint() {
	return static_cast<int>(m_imgNtHeaders.OptionalHeader.AddressOfEntryPoint);
}

std::uintptr_t CE::PEImage::getAddress() {
	return m_imgNtHeaders.OptionalHeader.ImageBase;
}

void CE::PEImage::parse() {
	std::vector<uint8_t> imgDosHeader(sizeof IMAGE_DOS_HEADER);
	m_reader->read(0x0, imgDosHeader);
	m_imgDosHeader = *reinterpret_cast<IMAGE_DOS_HEADER*>(imgDosHeader.data());
	
	const auto e_magic = reinterpret_cast<char*>(&m_imgDosHeader.e_magic);
	if (std::string(e_magic, 2) != "MZ")
		throw std::exception();

	std::vector<uint8_t> imgNtHeaders(sizeof IMAGE_NT_HEADERS);
	m_reader->read(m_imgDosHeader.e_lfanew, imgNtHeaders);
	m_imgNtHeaders = *reinterpret_cast<IMAGE_NT_HEADERS*>(imgNtHeaders.data());

	const auto signature = reinterpret_cast<char*>(&m_imgNtHeaders.Signature);
	if (std::string(signature, 2) != "PE")
		throw std::exception();
}

void CE::PEImage::loadSections()
{
	ImageSection headerSection;
	headerSection.m_name = "PE HEADER";
	headerSection.m_virtualSize = m_imgDosHeader.e_lfanew + sizeof(IMAGE_NT_HEADERS);
	m_imageSections.push_back(headerSection);

	for (int i = 0; i < m_imgNtHeaders.FileHeader.NumberOfSections; i++)
	{
		std::vector<uint8_t> imgSecHeader(sizeof IMAGE_SECTION_HEADER);
		m_reader->read(m_imgDosHeader.e_lfanew + sizeof(IMAGE_NT_HEADERS) + sizeof(IMAGE_SECTION_HEADER) * i, imgSecHeader);
		const auto pSeh = reinterpret_cast<IMAGE_SECTION_HEADER*>(imgSecHeader.data());

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
