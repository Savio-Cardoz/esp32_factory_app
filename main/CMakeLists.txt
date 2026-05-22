idf_component_register(SRCS "main.cpp" "update.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES sdcard_manager app_update)
