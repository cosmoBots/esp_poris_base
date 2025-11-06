# CMakeLists.txt template for OtCoap

# --- Variant gating via environment variable -------------------------------
# Expect an env var PORIS_ENABLE_<UPPER_NAME>=1 to include this component.
# We derive the uppercase name from "OtCoap" at configure time.
set(_COMP_NAME "OtCoap")
string(TOUPPER "${_COMP_NAME}" _COMP_UPPER)

if(NOT DEFINED ENV{PORIS_ENABLE_${_COMP_UPPER}})
  message(STATUS "[${_COMP_NAME}] disabled by variant (PORIS_ENABLE_${_COMP_UPPER} not set)")
  return()
endif()

# --- Normal component registration -----------------------------------------
idf_component_register(
  SRCS "OtCoap.c"
  INCLUDE_DIRS "include"
)

# --- Optional: tie to Kconfig switch ---------------------------------------
# If you also want a Kconfig-controlled compile-time flag, keep this block.
# (Your generator should normalize OtCoap to a valid Kconfig symbol)
if(CONFIG_PORIS_ENABLE_OtCoap)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE "PORIS_OtCoap_ENABLED=1")
endif()
