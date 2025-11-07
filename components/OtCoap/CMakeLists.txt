# CMakeLists.txt template for OtCoap

# Señales de activación:
# - CMake var: PORIS_ENABLE_OTCOAP (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_OTCOAP (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_OTCOAP OR DEFINED ENV{PORIS_ENABLE_OTCOAP}))
  message(STATUS "[ OtCoap ] disabled (no PORIS_ENABLE_OTCOAP)")
  return()
endif()

idf_component_register(
  SRCS "OtCoap.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg
)

if(CONFIG_OTCOAP_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_OTCOAP)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_OTCOAP_ENABLED=1")
endif()
