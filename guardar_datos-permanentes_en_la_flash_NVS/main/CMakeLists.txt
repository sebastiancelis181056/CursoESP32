idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES leds nvs_flash)
list(APPEND EXTRA_COMPONENT_DIRS
    components
)