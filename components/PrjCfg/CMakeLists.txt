# CMakeLists.txt template for PrjCfg

# Señales de activación:
# - CMake var: PORIS_ENABLE_PRJCFG (preferida, desde buildcfg/<Var>.components.cmake)
# - Env var  : PORIS_ENABLE_PRJCFG (compat para casos donde exportes env manualmente)
if(NOT (PORIS_ENABLE_PRJCFG OR DEFINED ENV{PORIS_ENABLE_PRJCFG}))
  message(STATUS "[ PrjCfg ] disabled (no PORIS_ENABLE_PRJCFG)")
  return()
endif()

idf_component_register(
  SRCS "PrjCfg.c"
  INCLUDE_DIRS "include"
)

# --- Optional: tie to Kconfig switch (symbol is uppercase) ------------------
# If Kconfig is used, define a macro for conditional code.
if(CONFIG_PORIS_ENABLE_PRJCFG)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_PRJCFG_ENABLED=1")
endif()
