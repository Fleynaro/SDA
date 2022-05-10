#pragma once
#include "Database.h"

namespace sda
{
    std::unique_ptr<Schema> GetSchema() {
        return std::make_unique<Schema>({
            {"functions", {}}
        });
    }
};