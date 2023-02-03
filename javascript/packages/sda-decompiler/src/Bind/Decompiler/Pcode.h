#pragma once
#include "SDA/Decompiler/Pcode/PcodeGraphBuilder.h"

namespace sda::bind
{
    class PcodeGraphBuilderBind
    {
        static auto New(pcode::Graph* graph, Image* image, std::shared_ptr<PcodeDecoder> decoder) {
            auto blockBuilder = std::make_shared<decompiler::PcodeBlockBuilder>(graph, image, decoder.get());
            return ExportObject(new decompiler::PcodeGraphBuilder(graph, blockBuilder));
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<decompiler::PcodeGraphBuilder>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .method("start", &decompiler::PcodeGraphBuilder::start)
                .static_method("New", &PcodeGraphBuilderBind::New);
            module.class_("PcodeGraphBuilder", cl);
        }
    };
};