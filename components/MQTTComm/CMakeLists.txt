# CMakeLists.txt template for MQTTComm

# Señales de activación:
# - CMake var: PORIS_ENABLE_MQTTCOMM (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_MQTTCOMM (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_MQTTCOMM OR DEFINED ENV{PORIS_ENABLE_MQTTCOMM}))
  message(STATUS "[ MQTTComm ] disabled (no PORIS_ENABLE_MQTTCOMM)")
  return()
endif()

idf_component_register(
  SRCS "MQTTComm_netvars.c" "MQTTComm.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg esp_wifi nvs_flash mqtt NetVars json
)

if(CONFIG_MQTTCOMM_USE_THREAD)
  target_link_libraries(${COMPONENT_LIB} PRIVATE idf::freertos)
endif()

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_MQTTCOMM)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_MQTTCOMM_ENABLED=1")
endif()
