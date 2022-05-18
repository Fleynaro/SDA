#pragma once
#include "Core/Object/ObjectList.h"
#include "Core/Offset.h"
#include "ImageReader.h"
#include "ImageAnalyser.h"

namespace sda
{
    struct ImageSection
	{
		enum SegmentType {
			NONE_SEGMENT,
			CODE_SEGMENT,
			DATA_SEGMENT
		};

		std::string m_name = "None";
		SegmentType m_type = NONE_SEGMENT;
		size_t m_relVirtualAddress = 0;
		size_t m_virtualSize = 0;
		size_t m_pointerToRawData = 0;

		Offset getMinOffset() const;

		Offset getMaxOffset() const;

		bool contains(Offset offset) const;

		// Image file offset to rva(=offset) (ghidra makes this transform automatically)
		Offset toOffset(size_t offset) const;

		// Rva(=offset) to image file offset (ghidra makes this transform automatically)
		size_t toImageFileOffset(Offset offset) const;
	};

    class ImageContext;

    class Image : public ContextObject
    {
        static inline const ImageSection DefaultSection = ImageSection();

        ImageContext* m_imageContext;
        std::unique_ptr<IImageReader> m_reader;
        std::shared_ptr<IImageAnalyser> m_analyser;
        std::uintptr_t m_baseAddress = 0;
        size_t m_entryPointOffset = 0;
        std::list<ImageSection> m_imageSections;

    public:
        static inline const std::string Collection = "images";

        Image(
            Context* context,
            std::unique_ptr<IImageReader> reader,
            std::shared_ptr<IImageAnalyser> analyser,
            ObjectId* id = nullptr,
            const std::string& name = "",
            ImageContext* imageContext = nullptr);

        void analyse();

        IImageReader* getReader() const;

        std::uintptr_t& getBaseAddress();

        size_t& getEntryPointOffset();

        std::list<ImageSection>& getImageSections();

        const ImageSection* getImageSectionAt(Offset offset) const;

        size_t getSize() const;

        bool contains(std::uintptr_t address) const;

        Offset toOffset(std::uintptr_t address) const;

        size_t toImageFileOffset(Offset offset) const;

        Image* clone(std::unique_ptr<IImageReader> reader) const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;
    };

    class ImageList : public ObjectList<Image>
    {
    public:
        using ObjectList<Image>::ObjectList;
    };
};