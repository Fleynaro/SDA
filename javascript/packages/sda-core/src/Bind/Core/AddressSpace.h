#include "SDA/Core/Image/AddressSpace.h"

namespace sda::bind
{
    class AddressSpaceBind : public ContextObjectBind
    {
        static auto New(Context* ctx, const std::string& name) {
            return new AddressSpace(ctx, nullptr, name);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<AddressSpace>(module);
            cl
                .inherit<ContextObject>()
                .property("images", &AddressSpace::getImages, &AddressSpace::setImages)
                .static_method("New", &New);
            module.class_("AddressSpace", cl);
        }
    };
};