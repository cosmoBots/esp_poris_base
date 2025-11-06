# --- Leer la lista desde el entorno y convertirla en LISTA real -------------
set(_raw "$ENV{PORIS_COMPONENTS_LIST}")         # e.g. "PrjCfg;OtCoap;HelloWorld"
message(STATUS "[main] PORIS_COMPONENTS_LIST='${_raw}'")
string(REPLACE "," ";" _raw "${_raw}")          # por si algún día viene con comas
string(REPLACE " " ";" _raw "${_raw}")          # por si viniera con espacios
separate_arguments(_PORIS_LIST UNIX_COMMAND "${_raw}")  # => lista real

# --- Construir REQUIRES de forma segura -------------------------------------
set(_reqs spi_flash)
list(APPEND _reqs ${_PORIS_LIST})
list(REMOVE_DUPLICATES _reqs)

message(STATUS "[main] REQUIRES(final)='${_reqs}'")

idf_component_register(
  SRCS "app_main.c"
  PRIV_REQUIRES spi_flash
  REQUIRES ${_reqs}
  INCLUDE_DIRS "."
)

