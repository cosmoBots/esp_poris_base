# CMakeLists.txt template for HelloWorld

# Señales de activación:
# - CMake var: PORIS_ENABLE_HELLOWORLD (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_HELLOWORLD (compat para casos donde exportes env manualmente)
if(NOT (PORIS_ENABLE_HELLOWORLD OR DEFINED ENV{PORIS_ENABLE_HELLOWORLD}))
  message(STATUS "[ HelloWorld ] disabled (no PORIS_ENABLE_HELLOWORLD)")
  return()
endif()

idf_component_register(
  SRCS "HelloWorld.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES PrjCfg
)

# --- Optional: tie to Kconfig switch (symbol is uppercase) ------------------
# If Kconfig is used, define a macro for conditional code.
if(CONFIG_PORIS_ENABLE_HELLOWORLD)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_HELLOWORLD_ENABLED=1")
endif()
