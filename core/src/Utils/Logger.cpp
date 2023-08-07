#include "SDA/Core/Utils/Logger.h"
#include <plog/Initializers/RollingFileInitializer.h>
#include <cstdlib>

void utils::InitLogger() {
    if (auto value = std::getenv("DISABLE_SDA_LOGS")) {
        if (std::string(value) == "1") {
            return;
        }
    }
    plog::init(plog::debug, "logs.txt");
}