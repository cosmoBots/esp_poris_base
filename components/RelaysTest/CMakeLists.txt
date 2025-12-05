# CMakeLists.txt template for RelaysTest

# Señales de activación:
# - CMake var: PORIS_ENABLE_RELAYSTEST (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_RELAYSTEST (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_RELAYSTEST OR DEFINED ENV{PORIS_ENABLE_RELAYSTEST}))
  message(STATUS "[ RelaysTest ] disabled (no PORIS_ENABLE_RELAYSTEST)")
  return()
endif()

idf_component_register(
  SRCS "RelaysTest.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg Relays
)

if(CONFIG_RELAYSTEST_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_RELAYSTEST)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_RELAYSTEST_ENABLED=1")
endif()
