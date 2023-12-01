#pragma once
#include "SDA/Core/Image/Image.h"

namespace sda::bind
{
    class ImageRWBind
	{
        static auto NewVectorImageRW(const std::vector<uint8_t>& data) {
            return new VectorImageRW(data);
        }

        static auto NewFileImageRW(const std::filesystem::path& pathToImgFile) {
            return new FileImageRW(pathToImgFile);
        }
    public:
        static void Init(v8pp::module& module) {
            {
                auto cl = NewClass<IImageRW>(module);
                cl
                    .auto_wrap_object_ptrs()
                    .method("read", std::function([](IImageRW* self, Offset offset, size_t size) {
                        std::vector<uint8_t> buffer(size);
                        self->readBytesAtOffset(offset, buffer);
                        return buffer;
                    }))
                    .method("write", &IImageRW::writeBytesAtOffset);
                module.class_("ImageRW", cl);
            }

            {
                auto cl = NewClass<VectorImageRW>(module);
                cl
                    .inherit<IImageRW>()
                    .property("size", &VectorImageRW::getImageSize)
                    .property("data", &VectorImageRW::getData)
                    .static_method("New", &NewVectorImageRW);
                module.class_("VectorImageRW", cl);
            }

            {
                auto cl = NewClass<FileImageRW>(module);
                cl
                    .inherit<IImageRW>()
                    //.inherit<utils::ISerializable>() TODO: implement multiple inheritance
                    .property("size", &FileImageRW::getImageSize)
                    .method("readFile", &FileImageRW::readFile)
                    .method("saveFile", &FileImageRW::saveFile)
                    .static_method("New", &NewFileImageRW);
                module.class_("FileImageRW", cl);
            }
        }
    };

    class ImageAnalyserBind
	{
        static auto NewPEImageAnalyser() {
            return std::make_shared<PEImageAnalyser>();
        }

        static auto NewTestAnalyser() {
            return std::make_shared<TestAnalyser>();
        }
    public:
        static void Init(v8pp::module& module) {
            {
                auto cl = NewClass<ImageAnalyser, v8pp::shared_ptr_traits>(module);
                cl
                    .auto_wrap_object_ptrs()
                    .property("name", &ImageAnalyser::getName)
                    .var("baseAddress", &ImageAnalyser::m_baseAddress)
                    .var("entryPointOffset", &ImageAnalyser::m_entryPointOffset)
                    .var("imageSections", &ImageAnalyser::m_imageSections)
                    .method("analyse", &ImageAnalyser::analyse);
                module.class_("ImageAnalyser", cl);
            }

            {
                auto cl = NewClass<PEImageAnalyser, v8pp::shared_ptr_traits>(module);
                cl
                    .inherit<ImageAnalyser>()
                    //.inherit<utils::ISerializable>() TODO: implement multiple inheritance
                    .static_method("New", &NewPEImageAnalyser);
                module.class_("PEImageAnalyser", cl);
            }

            {
                auto cl = NewClass<TestAnalyser, v8pp::shared_ptr_traits>(module);
                cl
                    .inherit<ImageAnalyser>()
                    //.inherit<utils::ISerializable>() TODO: implement multiple inheritance
                    .static_method("New", &NewTestAnalyser);
                module.class_("TestAnalyser", cl);
            }
        }
    };

    class ImageSectionBind
	{
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<ImageSection>(module);
            cl
                .auto_wrap_objects()
                .var("name", &ImageSection::m_name)
                .var("type", &ImageSection::m_type)
                .var("relVirtualAddress", &ImageSection::m_relVirtualAddress)
                .var("virtualSize", &ImageSection::m_virtualSize)
                .var("pointerToRawData", &ImageSection::m_pointerToRawData)
                .property("minOffset", &ImageSection::getMinOffset)
                .property("maxOffset", &ImageSection::getMaxOffset)
                .method("contains", &ImageSection::contains)
                .method("toOffset", &ImageSection::toOffset)
                .method("toImageFileOffset", &ImageSection::toImageFileOffset);
            module.class_("ImageSection", cl);
        }
    };

    class ImageBind : public ContextObjectBind
    {
        static auto New(
            Context* ctx,
            std::unique_ptr<IImageRW> rw,
            std::shared_ptr<ImageAnalyser> analyser,
            const std::string& name,
            SymbolTable* globalSymbolTable)
        {
            return new Image(
                ctx,
                std::move(rw),
                analyser,
                nullptr,
                name,
                globalSymbolTable);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<Image>(module);
            cl
                .inherit<ContextObject>()
                .property("rw", &Image::getRW)
                .property("baseAddress", &Image::getBaseAddress)
                .property("entryPointOffset", &Image::getEntryPointOffset)
                .property("imageSections", &Image::getImageSections)
                .property("size", &Image::getSize)
                .property("globalSymbolTable", &Image::getGlobalSymbolTable)
                .method("contains", &Image::contains)
                .method("toOffset", &Image::toOffset)
                .method("getImageSectionAt", &Image::getImageSectionAt)
                .method("analyse", &Image::analyse)
                .static_method("New", &New);
            RegisterClassName(cl, "Image");
            module.class_("Image", cl);
        }
    };

    static void ImageBindInit(v8pp::module& module) {
        ImageRWBind::Init(module);
        ImageAnalyserBind::Init(module);
        ImageSectionBind::Init(module);
        ImageBind::Init(module);
    }
};
