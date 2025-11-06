# CMakeLists.txt template for OtCoap

# --- Variant gating via environment variable (from variants.yml) ------------
# Expect PORIS_ENABLE_OTCOAP=1 in the environment/CMake include.
if(NOT DEFINED ENV{PORIS_ENABLE_OTCOAP})
  message(STATUS "[ OtCoap ] disabled by variant (PORIS_ENABLE_OTCOAP not set)")
  return()
endif()

idf_component_register(
  SRCS "OtCoap.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg
)

# --- Optional: tie to Kconfig switch (symbol is uppercase) ------------------
# If Kconfig is used, define a macro for conditional code.
if(CONFIG_PORIS_ENABLE_OTCOAP)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_OTCOAP_ENABLED=1")
endif()
