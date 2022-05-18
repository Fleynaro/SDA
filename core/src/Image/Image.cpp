#include "Core/Image/Image.h"
#include "Core/Context.h"

using namespace sda;

Offset ImageSection::getMinOffset() const {
    return m_relVirtualAddress;
}

Offset ImageSection::getMaxOffset() const {
    return m_relVirtualAddress + m_virtualSize;
}

bool ImageSection::contains(Offset offset) const {
    return offset >= getMinOffset() && offset < getMaxOffset();
}

Offset ImageSection::toOffset(size_t offset) const {
    return offset - m_pointerToRawData + m_relVirtualAddress;
}

size_t ImageSection::toImageFileOffset(Offset offset) const {
    return offset - m_relVirtualAddress + m_pointerToRawData;
}

Image::Image(
        Context* context,
        std::unique_ptr<IImageReader> reader,
        std::shared_ptr<IImageAnalyser> analyser,
        ObjectId* id,
        const std::string& name)
    : ContextObject(context, id, name), m_reader(std::move(reader)), m_analyser(analyser)
{
    m_context->getImages()->add(std::unique_ptr<Image>(this));
}

void Image::analyse() {
    m_analyser->analyse(this);
}

IImageReader* Image::getReader() const {
    return m_reader.get();
}

std::uintptr_t& Image::getBaseAddress() {
    return m_baseAddress;
}

size_t& Image::getEntryPointOffset() {
    return m_entryPointOffset;
}

std::list<ImageSection>& Image::getImageSections() {
    return m_imageSections;
}

const ImageSection* Image::getImageSectionAt(Offset offset) const {
    for (const auto& section : m_imageSections) {
        if (section.contains(offset)) {
            return &section;
        }
    }
    return &DefaultSection;
}

size_t Image::getSize() const {
    return m_reader->getImageSize();
}

bool Image::contains(std::uintptr_t address) const {
    return address >= m_baseAddress && address < m_baseAddress + getSize();
}

Offset Image::toOffset(std::uintptr_t address) const {
    return address - m_baseAddress;
}

size_t Image::toImageFileOffset(Offset offset) const {
	if (auto section = getImageSectionAt(offset))
		return section->toImageFileOffset(offset);
    return offset;
}

Image* Image::clone(std::unique_ptr<IImageReader> reader) const {
    auto clone = new Image(m_context, std::move(reader), m_analyser);
    boost::json::object data;
    serialize(data);
    clone->deserialize(data);
    return clone;
}

void Image::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);

    if(auto serReader = dynamic_cast<ISerializable*>(m_reader.get())) {
        boost::json::object readerData;
        serReader->serialize(readerData);
        data["reader"] = readerData;
    }

    if(auto serAnalyser = dynamic_cast<ISerializable*>(m_analyser.get())) {
        boost::json::object analyserData;
        serAnalyser->serialize(analyserData);
        data["analyser"] = analyserData;
    }
}

void Image::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);

    if(auto serReader = dynamic_cast<ISerializable*>(m_reader.get())) {
        serReader->deserialize(data["reader"].get_object());
    }

    if(auto serAnalyser = dynamic_cast<ISerializable*>(m_analyser.get())) {
        serAnalyser->deserialize(data["analyser"].get_object());
    }

    analyse();
}

void Image::destroy() {
    m_context->getImages()->remove(getId());
}