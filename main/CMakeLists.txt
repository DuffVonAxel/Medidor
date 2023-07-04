idf_component_register(SRCS "websocket.c" "websocket_server.c" "web_server.c" "web_client.c" "cJSON.c" "meterWiFi_C.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES "../html/error.html"
                    "../html/favicon.ico"
                    "../html/medidor.js"
                    "../html/fonts.css"
                    "../html/gauge.min.js"
                    "../html/index.html"
                    "../html/digital.ttf")
