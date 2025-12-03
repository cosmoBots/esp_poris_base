# CMakeLists.txt template for Measurement

# Señales de activación:
# - CMake var: PORIS_ENABLE_MEASUREMENT (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_MEASUREMENT (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_MEASUREMENT OR DEFINED ENV{PORIS_ENABLE_MEASUREMENT}))
  message(STATUS "[ Measurement ] disabled (no PORIS_ENABLE_MEASUREMENT)")
  return()
endif()

idf_component_register(
  SRCS "Measurement.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg
)

if(CONFIG_MEASUREMENT_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_MEASUREMENT)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_MEASUREMENT_ENABLED=1")
endif()
