# CMakeLists.txt template for Wifi

# Señales de activación:
# - CMake var: PORIS_ENABLE_WIFI (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_WIFI (compat si exportas env manualmente)

if(NOT (PORIS_ENABLE_WIFI OR DEFINED ENV{PORIS_ENABLE_WIFI}))
  message(STATUS "[ Wifi ] disabled (no PORIS_ENABLE_WIFI)")
  return()
endif()

idf_component_register(
  SRCS "Wifi_netvars.c" "Wifi.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg esp_wifi esp_netif esp_event nvs_flash NetVars json
)

# Si Kconfig lo activó, define una macro útil
if(CONFIG_PORIS_ENABLE_WIFI)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_WIFI_ENABLED=1")
endif()
