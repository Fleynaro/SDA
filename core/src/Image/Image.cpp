#include "Core/Image/Image.h"

using namespace sda;

Image::Image(
        Context* context,
        std::unique_ptr<IImageReader> reader,
        std::shared_ptr<IImageAnalyser> analyser,
        ObjectId* id,
        const std::string& name)
    : ContextObject(context, id, name), m_reader(std::move(reader)), m_analyser(analyser)
{}

void Image::analyse() {
    m_analyser->analyse(this);
}

bool Image::contains(std::uintptr_t address) const {
    return false;
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
}

void Image::destroy() {
    m_context->getImages()->remove(getId());
}

std::string Image::GetCollectionName() {
    return "images";
}