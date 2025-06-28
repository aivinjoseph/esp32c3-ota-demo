
idf_component_register(SRCS "ota_example.c" INCLUDE_DIRS ".")
target_compile_definitions(${COMPONENT_LIB} PRIVATE APP_VERSION=\"v1.0.1\")

