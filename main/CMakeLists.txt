idf_component_register(SRCS "hello_world_main.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS "")
                       
idf_component_optional_requires(PRIVATE OtCoap)  # en lugar de REQUIRES duro