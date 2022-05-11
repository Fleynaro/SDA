#pragma once
#include "Database.h"

namespace sda
{
    std::unique_ptr<Schema> GetSchema() {
        std::list<Schema::Collection> collections = {
            { "functions" }
        };
        return std::make_unique<Schema>(collections);
    }
};