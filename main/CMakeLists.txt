file(GLOB_RECURSE srcs
    "./**.cpp"
    "./**.hpp"
)

idf_component_register(SRCS "${srcs}"
    REQUIRES
    bt
    esp_driver_gpio
    nvs_flash
    INCLUDE_DIRS "."
)

target_precompile_headers(${COMPONENT_LIB} PRIVATE "./common.hpp")
