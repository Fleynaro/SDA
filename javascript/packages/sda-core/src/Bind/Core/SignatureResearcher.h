#pragma once
#include "SDA/Core/Researchers/SignatureResearcher.h"

using namespace sda::researcher;

namespace sda::bind
{
    class SignatureRepositoryBind
    {
        static auto New() {
            auto repo = new SignatureRepository();
            return ExportObject(repo);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<SignatureRepository>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .static_method("New", &New);
            RegisterClassName(cl, "SignatureRepository");
            module.class_("SignatureRepository", cl);
        }
    };

    class SignatureResearcherBind
    {
        static auto New(
            ircode::Program* program,
            std::shared_ptr<CallingConvention> callingConvention,
            SignatureRepository* signatureRepo
        ) {
            auto researcher = new SignatureResearcher(
                program,
                callingConvention,
                signatureRepo
            );
            return ExportObject(researcher);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<SignatureResearcher>(module);
            cl
                .auto_wrap_object_ptrs(true)
                .property("eventPipe", &SignatureResearcher::getEventPipe)
                .static_method("New", &New);
            RegisterClassName(cl, "SignatureResearcher");
            module.class_("SignatureResearcher", cl);
        }
    };

    static void SignatureBindInit(v8pp::module& module) {
        SignatureRepositoryBind::Init(module);
        SignatureResearcherBind::Init(module);
    }
};