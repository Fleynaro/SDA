
file(GLOB_RECURSE DATA_RSCS
    "*.sql")

cmrc_add_resource_library(resources ${DATA_RSCS})