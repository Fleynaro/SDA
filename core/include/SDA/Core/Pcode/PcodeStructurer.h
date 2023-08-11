#pragma once
#include "PcodeFunctionGraph.h"
#include "SDA/Core/Pcode/PcodePrinter.h"

namespace sda::pcode
{
    enum class GotoType {
        Default,
        Continue,
        Break
    };

    class StructBlock {
        friend class StructTree;
        friend class Structurer;
        Block* m_pcodeBlock;
        StructBlock* m_nearNextBlock = nullptr;
        StructBlock* m_farNextBlock = nullptr;
        std::list<StructBlock*> m_referencedBlocks;
        StructBlock* m_goto = nullptr;
        GotoType m_gotoType = GotoType::Default;
        std::list<StructBlock*> m_gotoReferencedBlocks;
        StructBlock* m_parent = nullptr;
    protected:
        bool m_embedded = false;
    public:
        StructBlock(Block* pcodeBlock = nullptr);

        std::string getName();

        Block* getPcodeBlock() const;

        void setNearNextBlock(StructBlock* nearNextBlock);

        StructBlock* getNearNextBlock() const;

        void setFarNextBlock(StructBlock* farNextBlock);

        StructBlock* getFarNextBlock() const;

        std::list<StructBlock*> getNextBlocks() const;

        void setGoto(StructBlock* gotoBlock, GotoType type);

        StructBlock* getGoto() const;

        GotoType getGotoType() const;

        const std::list<StructBlock*>& getGotoReferencedBlocks() const;

        void moveNextBlocksTo(StructBlock* block);

        const std::list<StructBlock*>& getReferencedBlocks() const;

        void moveReferencedBlocksTo(StructBlock* block);

        void setParent(StructBlock* block);

        StructBlock* getParent() const;

        StructBlock* getTop();

        StructBlock* getBottom();

        virtual void getLeafs(std::list<StructBlock*>& leafs);

        virtual void embed() {}
    };

    class StructBlockSequence : public StructBlock {
        std::list<StructBlock*> m_blocks;
    public:
        StructBlockSequence();

        void addBlock(StructBlock* block);

        const std::list<StructBlock*>& getBlocks() const;

        void getLeafs(std::list<StructBlock*>& leafs) override;

        void embed() override;
    };

    class StructBlockIf : public StructBlock {
        StructBlock* m_condBlock = nullptr;
        StructBlock* m_thenBlock = nullptr;
        StructBlock* m_elseBlock = nullptr;
        bool m_inverted = false;
    public:
        StructBlockIf();

        void setCondBlock(StructBlock* condBlock);

        StructBlock* getCondBlock() const;

        void setThenBlock(StructBlock* thenBlock);

        StructBlock* getThenBlock() const;

        void setElseBlock(StructBlock* elseBlock);

        StructBlock* getElseBlock() const;

        void setInverted(bool inverted);

        bool isInverted() const;

        void getLeafs(std::list<StructBlock*>& leafs) override;

        void embed() override;
    };

    class StructBlockWhile : public StructBlock {
        StructBlock* m_bodyBlock = nullptr;
    public:
        StructBlockWhile();

        void setBodyBlock(StructBlock* bodyBlock);

        StructBlock* getBodyBlock() const;

        void getLeafs(std::list<StructBlock*>& leafs) override;

        void embed() override;
    };

    class StructTree
    {
        friend class Structurer;
        std::list<std::unique_ptr<StructBlock>> m_blocks;
        StructBlock* m_entryBlock = nullptr;
    public:
        StructTree();

        StructBlock* getEntryBlock() const;

        void addBlock(std::unique_ptr<StructBlock> block);

        void clear();
    };

    class Structurer
    {
        StructTree* m_tree;
        FunctionGraph* m_funcGraph;
        struct BlockInfo {
            size_t index = 0;
            size_t level = 0;
            size_t backLevel = 0;
        };
        std::map<StructBlock*, BlockInfo> m_blockToInfo;
        std::list<StructBlock*> m_blocksToProcess;
        std::map<StructBlock*, utils::BitSet> m_dominants;

        struct LoopInfo {
            // see for details https://photos.app.goo.gl/XXEWd9PiCKrvnb2W6
            StructBlock* startBlock = nullptr;
            StructBlock* lastContinueBlock = nullptr;
            StructBlock* exitBlock = nullptr;
            std::set<StructBlock*> bodyBlocks;
        };
        std::list<LoopInfo> m_loops;
        std::map<StructBlock*, LoopInfo*> m_blockToLoop;

        struct Pattern {
            std::list<std::unique_ptr<StructBlock>> newBlocks;
            StructBlock* startBlock = nullptr;
            StructBlock* nearNextBlock = nullptr;
            StructBlock* farNextBlock = nullptr;
            std::list<StructBlock*> blocksToReplace;
            std::function<void()> customProcessing; // used only in TryReplaceWithGotoPattern yet
            size_t score = 0;
        };
    public:
        Structurer(FunctionGraph* funcGraph, StructTree* tree);

        void start();

    private:
        static bool TrySequencePattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern);

        static bool TryIfPattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern);

        static bool TryIfElsePattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern);

        static bool TryIfGotoPattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern);

        static bool TryReplaceWithGotoPattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern);

        static bool TryWhilePattern(Structurer* structurer, StructBlock* startBlock, Pattern& pattern);

        void applyPattern(Pattern& pattern);

        void initTree();

        void calculateBackLevel(StructBlock* block, std::list<StructBlock*>& path);

        void findLoops();

        void handleLoop(const LoopInfo& loopInfo);

        LoopInfo* getLoopAt(StructBlock* block);

        size_t getMinLevel(StructBlock* block);

        std::unique_ptr<StructBlockSequence> createSequenceBlock(const std::list<StructBlock*>& blocks);

        std::map<StructBlock*, utils::BitSet> getDominants(StructBlock* block);

        void passDescendants(StructBlock* startBlock, std::function<void(StructBlock* block, bool& goNextBlocks)> callback, bool ignoreLoop);

        std::list<StructBlock*> getReferencedBlocks(StructBlock* block, bool ignoreLoop);

        std::list<StructBlock*> getNextBlocks(StructBlock* block, bool ignoreLoop);
    };

    class StructTreePrinter : public utils::AbstractPrinter
    {
        using PrinterFunction = std::function<void(Block* block)>;
        PrinterFunction m_codePrinter;
        PrinterFunction m_conditionPrinter;
    public:
        StructTreePrinter();

        void setCodePrinter(PrinterFunction codePrinter);

        void setConditionPrinter(PrinterFunction conditionPrinter);

        void printStructBlock(StructBlock* block);

        void printStructBlockSequence(StructBlockSequence* block);

        static PrinterFunction CodePrinter(pcode::Printer* pcodePrinter);

        static PrinterFunction ConditionPrinter(pcode::Printer* pcodePrinter);
    };
};
