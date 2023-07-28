#include "SDA/Core/Image/Image.h"
#include "SDA/Core/SymbolTable/OptimizedSymbolTable.h"
#include "SDA/Core/Context.h"

using namespace sda;

Image::Image(
        Context* context,
        std::unique_ptr<IImageRW> rw,
        std::shared_ptr<ImageAnalyser> analyser,
        Object::Id* id,
        const std::string& name,
        SymbolTable* globalSymbolTable)
    : ContextObject(context, id, name),
    m_rw(std::move(rw)),
    m_analyser(analyser),
    m_globalSymbolTable(globalSymbolTable)
{
    m_context->getImages()->add(std::unique_ptr<Image>(this));
}

void Image::analyse() {
    m_analyser->analyse(getRW());
}

IImageRW* Image::getRW() const {
    return m_rw.get();
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
    return m_rw->getImageSize();
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

void Image::compare(Image* otherImage, std::list<std::pair<Offset, Offset>>& regions) const {
    if (getSize() != otherImage->getSize())
        throw std::runtime_error("Images have different sizes");

    for (const auto& section : getImageSections()) {
        if (section.m_type == ImageSection::CODE_SEGMENT) {
            auto offset = section.getMinOffset();
            auto otherSection = otherImage->getImageSectionAt(offset);
            if (!otherSection || otherSection->m_type != ImageSection::CODE_SEGMENT ||
                otherSection->getMinOffset() != section.getMinOffset() ||
                otherSection->getMaxOffset() != section.getMaxOffset())
                throw std::runtime_error("Images have different code sections");

            while (offset < section.getMaxOffset()) {
                // load blocks
                std::vector<uint8_t> block1(128);
                m_rw->readBytesAtOffset(offset, block1);
                std::vector<uint8_t> block2(block1.size());
                otherImage->m_rw->readBytesAtOffset(offset, block2);

                // compare blocks
                std::pair<Offset, Offset> region;
                bool isDifferenceFound = false;
                for (size_t i = 0; i < block1.size(); i++) {
                    if (block1[i] != block2[i]) {
                        region.first = offset + i;
                        isDifferenceFound = true;
                        break;
                    } else {
                        if (isDifferenceFound) {
                            region.second = offset + i;
                            regions.push_back(region);
                            isDifferenceFound = false;
                        }
                    }
                }

                offset += block1.size();
            }
        }
    }
}

Image* Image::clone(std::unique_ptr<IImageRW> rw) const {
    auto clone = new Image(m_context, std::move(rw), m_analyser);
    boost::json::object data;
    serialize(data);
    clone->deserialize(data);
    return clone;
}

void Image::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);
    data["collection"] = Class;

    if(auto serRW = dynamic_cast<utils::ISerializable*>(m_rw.get())) {
        boost::json::object rwData;
        serRW->serialize(rwData);
        data["rw"] = rwData;
    }

    if(auto serAnalyser = dynamic_cast<utils::ISerializable*>(m_analyser.get())) {
        boost::json::object analyserData;
        serAnalyser->serialize(analyserData);
        data["analyser"] = analyserData;
    }

    data["global_symbol_table"] = m_globalSymbolTable->serializeId();
}

void Image::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);

    if(auto serRW = dynamic_cast<utils::ISerializable*>(m_rw.get())) {
        serRW->deserialize(data["rw"].get_object());
    }

    if(auto serAnalyser = dynamic_cast<utils::ISerializable*>(m_analyser.get())) {
        serAnalyser->deserialize(data["analyser"].get_object());
        analyse();
    }

    m_globalSymbolTable = m_context->getSymbolTables()->get(data["global_symbol_table"]);
}

void Image::destroy() {
    m_context->getImages()->remove(this);
}