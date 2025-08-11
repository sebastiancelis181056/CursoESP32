idf_component_register(SRCS "main.c"
                    PRIV_REQUIRES spi_flash
                    REQUIRES driver fatfs sdmmc esp_driver_sdspi esp_driver_spi vfs
                    INCLUDE_DIRS ".")
