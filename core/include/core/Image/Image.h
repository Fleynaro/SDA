#pragma once
#include "Core/Object/ObjectList.h"
#include "ImageReader.h"
#include "ImageAnalyser.h"
#include "Core/Pcode/PcodeBlock.h"

namespace sda
{
    class SymbolTable;

    class Image : public ContextObject
    {
        std::unique_ptr<IImageReader> m_reader;
        std::shared_ptr<ImageAnalyser> m_analyser;
        SymbolTable* m_globalSymbolTable;
        std::map<pcode::InstructionOffset, pcode::Instruction> m_instructions;
        std::map<pcode::InstructionOffset, pcode::Block> m_blocks;

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

        std::map<pcode::InstructionOffset, pcode::Instruction>& getInstructions();

        std::map<pcode::InstructionOffset, pcode::Block>& getBlocks();

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