# CMakeLists.txt template for $$1

# --- Variant gating via environment variable (from variants.yml) ------------
# Expect PORIS_ENABLE_$#1=1 in the environment/CMake include.
if(NOT DEFINED ENV{PORIS_ENABLE_$#1})
  message(STATUS "[ $$1 ] disabled by variant (PORIS_ENABLE_$#1 not set)")
  return()
endif()

idf_component_register(
  SRCS "$$1.c"
  INCLUDE_DIRS "include"
)

# --- Optional: tie to Kconfig switch (symbol is uppercase) ------------------
# If Kconfig is used, define a macro for conditional code.
if(CONFIG_PORIS_ENABLE_$#1)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_$#1_ENABLED=1")
endif()
