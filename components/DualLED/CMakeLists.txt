# CMakeLists.txt template for DualLED

# Señales de activación:
# - CMake var: PORIS_ENABLE_DUALLED (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_DUALLED (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_DUALLED OR DEFINED ENV{PORIS_ENABLE_DUALLED}))
  message(STATUS "[ DualLED ] disabled (no PORIS_ENABLE_DUALLED)")
  return()
endif()

idf_component_register(
  SRCS "DualLED_netvars.c" "DualLED.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg esp_driver_gpio NetVars json nvs_flash
)

if(CONFIG_DUALLED_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_DUALLED)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_DUALLED_ENABLED=1")
endif()
