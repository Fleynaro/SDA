#include "Core/Image/Image.h"
#include "Core/SymbolTable/SymbolTable.h"
#include "Core/Context.h"

using namespace sda;

Image::Image(
        Context* context,
        std::unique_ptr<IImageReader> reader,
        std::shared_ptr<ImageAnalyser> analyser,
        Object::Id* id,
        const std::string& name,
        SymbolTable* globalSymbolTable)
    : ContextObject(context, id, name),
    m_reader(std::move(reader)),
    m_analyser(analyser),
    m_globalSymbolTable(globalSymbolTable)
{
    m_context->getImages()->add(std::unique_ptr<Image>(this));
    m_pcodeGraph = std::make_unique<pcode::Graph>(this);
}

void Image::analyse() {
    m_analyser->analyse(getReader());
}

IImageReader* Image::getReader() const {
    return m_reader.get();
}

std::uintptr_t Image::getBaseAddress() const {
    return m_analyser->m_baseAddress;
}

Offset Image::getEntryPointOffset() const {
    return m_analyser->m_entryPointOffset;
}

const std::list<ImageSection>& Image::getImageSections() const {
    return m_analyser->m_imageSections;
}

const ImageSection* Image::getImageSectionAt(Offset offset) const {
    for (const auto& section : getImageSections()) {
        if (section.contains(offset)) {
            return &section;
        }
    }
    return nullptr;
}

size_t Image::getSize() const {
    return m_reader->getImageSize();
}

bool Image::contains(std::uintptr_t address) const {
    return address >= getBaseAddress() && address < getBaseAddress() + getSize();
}

Offset Image::toOffset(std::uintptr_t address) const {
    return address - getBaseAddress();
}

size_t Image::toImageFileOffset(Offset offset) const {
	if (auto section = getImageSectionAt(offset))
		return section->toImageFileOffset(offset);
    return offset;
}

SymbolTable* Image::getGlobalSymbolTable() const {
    return m_globalSymbolTable;
}

pcode::Graph* Image::getPcodeGraph() const {
    return m_pcodeGraph.get();
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
    data["collection"] = Collection;

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

    data["global_symbol_table"] = m_globalSymbolTable->serializeId();
}

void Image::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);

    if(auto serReader = dynamic_cast<ISerializable*>(m_reader.get())) {
        serReader->deserialize(data["reader"].get_object());
    }

    if(auto serAnalyser = dynamic_cast<ISerializable*>(m_analyser.get())) {
        serAnalyser->deserialize(data["analyser"].get_object());
        analyse();
    }

    m_globalSymbolTable = m_context->getSymbolTables()->get(data["global_symbol_table"]);
}

void Image::destroy() {
    m_context->getImages()->remove(this);
}