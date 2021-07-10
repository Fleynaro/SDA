#include "PEImage.h"

CE::PEImage::PEImage(int8_t* data, int size)
	: m_data(data), m_size(size)
{
	parse();
	loadSections();
}

int8_t* CE::PEImage::getData() {
	return m_data;
}

int CE::PEImage::getSize() {
	return m_size;
}

int CE::PEImage::getOffsetOfEntryPoint() {
	return static_cast<int>(m_pImgNtHeaders->OptionalHeader.AddressOfEntryPoint);
}

std::uintptr_t CE::PEImage::getAddress() {
	return m_pImgNtHeaders->OptionalHeader.ImageBase;
}

void CE::PEImage::parse() {
	m_pImgDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(m_data);
	const auto e_magic = reinterpret_cast<char*>(&m_pImgDosHeader->e_magic);
	if (std::string(e_magic, 2) != "MZ")
		throw std::exception();

	m_pImgNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(m_data + m_pImgDosHeader->e_lfanew);

	const auto signature = reinterpret_cast<char*>(&m_pImgNtHeaders->Signature);
	if (std::string(signature, 2) != "PE")
		throw std::exception();

	m_pImgSecHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(m_data + m_pImgDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));
}

void CE::PEImage::loadSections()
{
	ImageSection headerSection;
	headerSection.m_name = "PE HEADER";
	headerSection.m_virtualSize = m_pImgDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS);
	m_imageSections.push_back(headerSection);
	
	size_t i = 0;
	PIMAGE_SECTION_HEADER pSeh = m_pImgSecHeader;
	for (i = 0; i < m_pImgNtHeaders->FileHeader.NumberOfSections; i++)
	{
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
		pSeh++;
	}
}
