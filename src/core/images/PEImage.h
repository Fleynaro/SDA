#pragma once
#include "IImage.h"

namespace CE
{
	class PEImage : public IImage
	{
		byte* m_data;
		int m_size;
		PIMAGE_NT_HEADERS m_pImgNtHeaders;
		PIMAGE_SECTION_HEADER m_pImgSecHeader;
	public:
		PEImage(byte* data, int size)
			: m_data(data), m_size(size)
		{
			parse();
		}

		byte* getData() override {
			return m_data;
		}

		int getSize() override {
			return m_size;
		}

		int getOffsetOfEntryPoint() override {
			return (int)m_pImgNtHeaders->OptionalHeader.AddressOfEntryPoint;
		}

		SegmentType defineSegment(int offset) override {
			auto pSeh = defineSection(offset);
			auto name = std::string((char*)pSeh->Name);
			if (name == ".text")
				return CODE_SEGMENT;
			if (name == ".data" || name == ".rdata")
				return DATA_SEGMENT;
			return NONE_SEGMENT;
		}

		// rva to file offset (ghidra makes this transform automatically)
		int toImageOffset(int offset) override {
			if (offset == 0)
				return offset;
			auto pSeh = defineSection(offset);
			return (offset - pSeh->VirtualAddress + pSeh->PointerToRawData);
		}

		// virtual address to file offset
		int addrToImageOffset(uint64_t addr) override {
			auto offset = int(addr - getAddress());
			return toImageOffset(offset);
		}

		std::uintptr_t getAddress() override {
			return m_pImgNtHeaders->OptionalHeader.ImageBase;
		}

	private:
		PIMAGE_SECTION_HEADER defineSection(DWORD rva) {
			size_t i = 0;
			PIMAGE_SECTION_HEADER pSeh = m_pImgSecHeader;
			for (i = 0; i < m_pImgNtHeaders->FileHeader.NumberOfSections; i++) {
				if (rva >= pSeh->VirtualAddress && rva < pSeh->VirtualAddress +
					pSeh->Misc.VirtualSize) {
					break;
				}
				pSeh++;
			}
			return pSeh;
		}

		void parse() {
			auto& dos_header = *(IMAGE_DOS_HEADER*)m_data;
			auto e_magic = (char*)&dos_header.e_magic;
			if (std::string(e_magic, 2) != "MZ")
				throw std::exception();

			m_pImgNtHeaders = (PIMAGE_NT_HEADERS)(m_data + dos_header.e_lfanew);

			auto signature = (char*)&m_pImgNtHeaders->Signature;
			if (std::string(signature, 2) != "PE")
				throw std::exception();

			m_pImgSecHeader = (PIMAGE_SECTION_HEADER)(m_data + dos_header.e_lfanew + sizeof(IMAGE_NT_HEADERS));
		}
	};
};