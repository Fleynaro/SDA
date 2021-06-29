#pragma once
#include "main.h"

namespace CE
{
	// raw image that can manipualtes with bytes, no symbols or other high-level things
	class IImage {
	public:
		enum SegmentType {
			NONE_SEGMENT,
			CODE_SEGMENT,
			DATA_SEGMENT
		};

		virtual byte* getData() = 0;

		virtual int getSize() = 0;

		virtual int getOffsetOfEntryPoint() = 0;

		virtual int toImageOffset(int offset) {
			return offset;
		}

		virtual int addrToImageOffset(uint64_t addr) {
			throw std::logic_error("not implemented");
		}

		virtual SegmentType defineSegment(int offset) {
			return NONE_SEGMENT;
		}

		virtual std::uintptr_t getAddress() {
			return 0x0;
		}
	};
};