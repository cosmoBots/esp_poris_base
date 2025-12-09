# CMakeLists.txt template for UDPComm

# Señales de activación:
# - CMake var: PORIS_ENABLE_UDPCOMM (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_UDPCOMM (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_UDPCOMM OR DEFINED ENV{PORIS_ENABLE_UDPCOMM}))
  message(STATUS "[ UDPComm ] disabled (no PORIS_ENABLE_UDPCOMM)")
  return()
endif()

idf_component_register(
  SRCS "UDPComm.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg
)

if(CONFIG_UDPCOMM_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_UDPCOMM)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_UDPCOMM_ENABLED=1")
endif()
