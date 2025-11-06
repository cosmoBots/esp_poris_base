# main/CMakeLists.txt

# 1) Lee SIEMPRE del entorno
set(_env "$ENV{PORIS_COMPONENTS_LIST}")

# 2) Normaliza a lista: admite comas y espacios, conviértelos a ';'
set(_raw "${_env}")
string(REPLACE "," ";" _raw "${_raw}")
string(REPLACE " " ";" _raw "${_raw}")

# 3) Asigna como **lista real** (CMake parte por ';')
set(_PORIS_LIST ${_raw})

# 4) Mensajes de diagnóstico
message(STATUS "[main] ENV{PORIS_COMPONENTS_LIST}='${_env}'")
message(STATUS "[main] _PORIS_LIST='${_PORIS_LIST}'")

# 5) Registra el componente usando REQUIRES con lista real
idf_component_register(
  SRCS "app_main.c"
  PRIV_REQUIRES spi_flash
  REQUIRES ${_PORIS_LIST}
  INCLUDE_DIRS "."
)

# Mantén esto en comentario como pediste:
# idf_component_optional_requires(PRIVATE ${_PORIS_LIST})

# (Opcional) Salvaguarda si ya existen (ahora **sí** iterará bien)
foreach(_c IN LISTS _PORIS_LIST)
  if(TARGET ${_c})
    target_include_directories(${COMPONENT_LIB} PRIVATE
      $<TARGET_PROPERTY:${_c},INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(${COMPONENT_LIB} PRIVATE ${_c})
  endif()
endforeach()
