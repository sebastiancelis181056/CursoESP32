idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "view.html"
                    REQUIRES esp_http_server esp_wifi)
