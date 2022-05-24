#pragma once
#include "Core/Object/ObjectList.h"
#include "ImageReader.h"
#include "ImageAnalyser.h"
#include "Core/Pcode/PcodeGraph.h"

namespace sda
{
    class SymbolTable;

    class Image : public ContextObject
    {
        std::unique_ptr<IImageReader> m_reader;
        std::shared_ptr<ImageAnalyser> m_analyser;
        SymbolTable* m_globalSymbolTable;
        std::unique_ptr<pcode::Graph> m_pcodeGraph;

    public:
        static inline const std::string Collection = "images";

        Image(
            Context* context,
            std::unique_ptr<IImageReader> reader,
            std::shared_ptr<ImageAnalyser> analyser,
            Object::Id* id = nullptr,
            const std::string& name = "",
            SymbolTable* globalSymbolTable = nullptr);

        void analyse();

        IImageReader* getReader() const;

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