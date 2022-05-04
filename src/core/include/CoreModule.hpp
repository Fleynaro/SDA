#pragma once
#include <Module.hpp>

namespace sda
{
    class CoreModule : public IModule
    {
    public:
        std::string getName() {
            return "Core";
        }

        boost::dll::fs::path location() const override {
            return boost::dll::this_line_location();
        }

        void init(Program* program) override {
            
        }

        static std::unique_ptr<IModule> Create() {
            return std::make_unique<CoreModule>();
        }
    };
};

EXPORT_MODULE(sda::CoreModule)