#include "SimpleImage.h"

CE::SimpleImage::SimpleImage(IReader* reader, int size)
	: AbstractImage(reader), m_size(size)
{}

int CE::SimpleImage::getOffsetOfEntryPoint() {
	return 0;
}
