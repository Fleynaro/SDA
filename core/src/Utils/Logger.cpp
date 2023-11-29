#include "SDA/Core/Utils/Logger.h"
#include <plog/Initializers/RollingFileInitializer.h>
#include <cstdlib>
#include <filesystem>

void utils::InitLogger() {
    if (auto value = std::getenv("DISABLE_SDA_LOGS")) {
        if (std::string(value) == "1") {
            return;
        }
    }
    std::filesystem::path path = "logs.txt";
    if (auto value = std::getenv("SDA_LOGS_PATH")) {
        path = std::filesystem::path(value);
    }
    plog::init(plog::debug, path.c_str());
}