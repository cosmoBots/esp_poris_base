# CMakeLists.txt template for Relays

# Señales de activación:
# - CMake var: PORIS_ENABLE_RELAYS (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_RELAYS (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_RELAYS OR DEFINED ENV{PORIS_ENABLE_RELAYS}))
  message(STATUS "[ Relays ] disabled (no PORIS_ENABLE_RELAYS)")
  return()
endif()

idf_component_register(
  SRCS "Relays.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg esp_driver_gpio
)

if(CONFIG_RELAYS_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_RELAYS)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_RELAYS_ENABLED=1")
endif()
