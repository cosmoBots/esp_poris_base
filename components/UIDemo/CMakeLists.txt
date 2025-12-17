# CMakeLists.txt template for UIDemo

# Señales de activación:
# - CMake var: PORIS_ENABLE_UIDEMO (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_UIDEMO (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_UIDEMO OR DEFINED ENV{PORIS_ENABLE_UIDEMO}))
  message(STATUS "[ UIDemo ] disabled (no PORIS_ENABLE_UIDEMO)")
  return()
endif()

idf_component_register(
  SRCS "UIDemo_netvars.c" "UIDemo.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg NetVars json nvs_flash TouchScreen esp_lcd
)

if(CONFIG_UIDEMO_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_UIDEMO)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_UIDEMO_ENABLED=1")
endif()
