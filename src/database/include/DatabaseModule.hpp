#pragma once
#include <Core.hpp>
#include <Module.hpp>

namespace sda
{
    class DatabaseModule : public IModule
    {
    public:
        std::string getName();

        boost::dll::fs::path location() const override {
            return boost::dll::this_line_location();
        }

        void init(Program* program) override {
            
        }

        static std::unique_ptr<IModule> Create() {
            return std::make_unique<DatabaseModule>();
        }
    };
};

EXPORT_MODULE(sda::DatabaseModule)