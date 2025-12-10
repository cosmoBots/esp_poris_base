# CMakeLists.txt template for LcdScreen

# Señales de activación:
# - CMake var: PORIS_ENABLE_LCDSCREEN (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_LCDSCREEN (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_LCDSCREEN OR DEFINED ENV{PORIS_ENABLE_LCDSCREEN}))
  message(STATUS "[ LcdScreen ] disabled (no PORIS_ENABLE_LCDSCREEN)")
  return()
endif()

idf_component_register(
  SRCS "LcdScreen_netvars.c" "LcdScreen.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg NetVars json nvs_flash
)

if(CONFIG_LCDSCREEN_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_LCDSCREEN)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_LCDSCREEN_ENABLED=1")
endif()
