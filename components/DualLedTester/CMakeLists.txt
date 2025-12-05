# CMakeLists.txt template for DualLedTester

# Señales de activación:
# - CMake var: PORIS_ENABLE_DUALLEDTESTER (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_DUALLEDTESTER (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_DUALLEDTESTER OR DEFINED ENV{PORIS_ENABLE_DUALLEDTESTER}))
  message(STATUS "[ DualLedTester ] disabled (no PORIS_ENABLE_DUALLEDTESTER)")
  return()
endif()

idf_component_register(
  SRCS "DualLedTester.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg DualLED
)

if(CONFIG_DUALLEDTESTER_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_DUALLEDTESTER)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_DUALLEDTESTER_ENABLED=1")
endif()
