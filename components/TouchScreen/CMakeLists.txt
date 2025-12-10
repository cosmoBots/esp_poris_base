# CMakeLists.txt template for TouchScreen

# Señales de activación:
# - CMake var: PORIS_ENABLE_TOUCHSCREEN (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_TOUCHSCREEN (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_TOUCHSCREEN OR DEFINED ENV{PORIS_ENABLE_TOUCHSCREEN}))
  message(STATUS "[ TouchScreen ] disabled (no PORIS_ENABLE_TOUCHSCREEN)")
  return()
endif()

idf_component_register(
  SRCS "TouchScreen_netvars.c" "TouchScreen.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg NetVars json nvs_flash
)

if(CONFIG_TOUCHSCREEN_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_TOUCHSCREEN)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_TOUCHSCREEN_ENABLED=1")
endif()
