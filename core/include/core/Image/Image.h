#pragma once
#include "Core/Object/ObjectList.h"
#include "ImageRW.h"
#include "ImageAnalyser.h"
#include "Core/Pcode/PcodeGraph.h"

namespace sda
{
    class SymbolTable;

    class Image : public ContextObject
    {
        std::unique_ptr<IImageRW> m_rw;
        std::shared_ptr<ImageAnalyser> m_analyser;
        SymbolTable* m_globalSymbolTable;
        std::unique_ptr<pcode::Graph> m_pcodeGraph;

    public:
        static inline const std::string Collection = "images";

        Image(
            Context* context,
            std::unique_ptr<IImageRW> rw,
            std::shared_ptr<ImageAnalyser> analyser,
            Object::Id* id = nullptr,
            const std::string& name = "",
            SymbolTable* globalSymbolTable = nullptr);

        void analyse();

        IImageRW* getRW() const;

        std::uintptr_t getBaseAddress() const;

        Offset getEntryPointOffset() const;

        const std::list<ImageSection>& getImageSections() const;

        const ImageSection* getImageSectionAt(Offset offset) const;

        size_t getSize() const;

        bool contains(std::uintptr_t address) const;

        Offset toOffset(std::uintptr_t address) const;

        size_t toImageFileOffset(Offset offset) const;

        SymbolTable* getGlobalSymbolTable() const;

        pcode::Graph* getPcodeGraph() const;

        // compare this image with another image on code sections
        void compare(Image* otherImage, std::list<std::pair<Offset, Offset>>& regions) const;

        Image* clone(std::unique_ptr<IImageRW> rw) const;

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