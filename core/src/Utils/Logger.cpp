#include "SDA/Core/Utils/Logger.h"
#include <plog/Initializers/RollingFileInitializer.h>

void utils::InitLogger() {
    plog::init(plog::debug, "logs.txt");
}