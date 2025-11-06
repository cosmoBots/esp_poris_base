cmake_minimum_required(VERSION 3.16)

# --- Entrada de usuario (opcional por CLI o variables de entorno) ---
#   -DVARIANT=NombreDeVariante   (p.ej., S3LCDWifi)
#   -DIDF_TARGET=esp32s3         (normalmente NO hace falta si usas gen_variant.py)
if(NOT DEFINED VARIANT AND DEFINED ENV{VARIANT})
  set(VARIANT "$ENV{VARIANT}")
endif()

# Rutas base
set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")
set(BUILDCFG_DIR  "${PROJECT_ROOT}/buildcfg")

# --- Cargar cache/flags de la variante -------------------------------------
if(VARIANT)
  set(VAR_CACHE   "${BUILDCFG_DIR}/${VARIANT}.cmakecache.cmake")
  set(VAR_SDKDEFS "${BUILDCFG_DIR}/sdkconfig.${VARIANT}.defaults")
  set(VAR_COMP    "${BUILDCFG_DIR}/${VARIANT}.components.cmake")

  if(EXISTS "${VAR_COMP}")
    message(STATUS "Incluyendo flags de componentes: ${VAR_COMP}")
    include("${VAR_COMP}")    # define: set(PORIS_COMPONENTS PrjCfg OtCoap) + PORIS_ENABLE_*
  else()
    set(PORIS_COMPONENTS "")  # sin componentes declarados
  endif()

  if(EXISTS "${VAR_CACHE}")
    message(STATUS "Incluyendo cache de variante: ${VAR_CACHE}")
    include("${VAR_CACHE}")   # suele fijar IDF_TARGET, etc.
  else()
    message(FATAL_ERROR
      "No existe ${VAR_CACHE}.\n"
      "Genera la variante primero: python3 tools/gen_variant.py --variant ${VARIANT}")
  endif()

  if(EXISTS "${VAR_SDKDEFS}")
    # IDF admite una lista (separada por ';'), aquí 1 fichero.
    set(SDKCONFIG_DEFAULTS "${VAR_SDKDEFS}" CACHE STRING "sdkconfig defaults for variant" FORCE)
    message(STATUS "Usando SDKCONFIG_DEFAULTS=${SDKCONFIG_DEFAULTS}")
  else()
    message(FATAL_ERROR
      "No existe ${VAR_SDKDEFS}.\n"
      "Genera la variante: python3 tools/gen_variant.py --variant ${VARIANT}")
  endif()
else()
  message(STATUS "VARIANT no establecido. Usaré sdkconfig.defaults (si existe).")
endif()

# --- IDF target -------------------------------------------------------------
# Prioridad: cache de variante -> CLI/ENV -> existente
if(NOT DEFINED IDF_TARGET AND DEFINED ENV{IDF_TARGET})
  set(IDF_TARGET "$ENV{IDF_TARGET}")
endif()
if(DEFINED IDF_TARGET)
  message(STATUS "IDF_TARGET=${IDF_TARGET}")
else()
  message(WARNING
    "IDF_TARGET no definido explícitamente. Si tu variante no lo fijó, "
    "considera pasar -DIDF_TARGET=esp32c6|esp32c5|esp32s3, o añade 'target' al YAML.")
endif()

# --- Normalización de componentes y override opcional -----------------------
#   Fuente de verdad: PORIS_COMPONENTS desde buildcfg/<Var>.components.cmake
#   Override opcional: ENV{PORIS_COMPONENTS_OVERRIDE} (formato "A,B" o "A;B")
set(_raw "${PORIS_COMPONENTS}")

if(DEFINED ENV{PORIS_COMPONENTS_OVERRIDE} AND NOT "$ENV{PORIS_COMPONENTS_OVERRIDE}" STREQUAL "")
  set(_ovr "$ENV{PORIS_COMPONENTS_OVERRIDE}")
  string(REPLACE "," ";" _ovr "${_ovr}")      # admite comas
  if(_raw)
    list(APPEND _raw ${_ovr})
  else()
    set(_raw "${_ovr}")
  endif()
endif()

# Convierte cualquier string potencial a LISTA real (sin cachearla como STRING)
# 1) espacios -> ';' (por si alguien mete "A B")
string(REPLACE " " ";" _raw "${_raw}")
# 2) tokeniza respetando ';' como separador de lista
separate_arguments(PORIS_COMPONENTS_LIST UNIX_COMMAND "${_raw}")
list(REMOVE_DUPLICATES PORIS_COMPONENTS_LIST)

message(STATUS "PORIS_COMPONENTS_LIST inicial = ${PORIS_COMPONENTS_LIST}")

# --- Exporta la lista para TODAS las pasadas/directorios ---
# a) Cache: persiste entre re-configures
set(PORIS_COMPONENTS_LIST "${PORIS_COMPONENTS_LIST}"
    CACHE STRING "PORIS components list for this variant" FORCE)

# b) Propiedad GLOBAL: accesible desde subdirectorios aunque la var normal esté vacía
set_property(GLOBAL PROPERTY PORIS_COMPONENTS_LIST "${PORIS_COMPONENTS_LIST}")

message(STATUS "PORIS_COMPONENTS_LIST global = ${PORIS_COMPONENTS_LIST}")

# Exporta a caché la lista y el fichero para que main pueda “rehidratarse”
set(PORIS_COMPONENTS_LIST "${PORIS_COMPONENTS_LIST}"
    CACHE STRING "PORIS components list for this variant" FORCE)

set(PORIS_COMPONENTS_FILE "${_VARCOMP}"
    CACHE FILEPATH "CMake include file that defines PORIS_COMPONENTS" FORCE)

message(STATUS "PORIS_COMPONENTS_LIST inicial = ${PORIS_COMPONENTS_LIST}")
message(STATUS "PORIS_COMPONENTS_FILE = ${PORIS_COMPONENTS_FILE}")

# Normaliza la lista a formato con ';'
set(_raw_list "${PORIS_COMPONENTS_LIST}")
string(REPLACE "," ";" _raw_list "${_raw_list}")

# Exporta al entorno para TODAS las pasadas
set(ENV{PORIS_COMPONENTS_LIST} "${_raw_list}")

# (Opcional) también a caché por si quieres inspeccionarla desde cmake-gui/ninja reconfigure
set(PORIS_COMPONENTS_LIST "${_raw_list}" CACHE STRING "PORIS components list for this variant" FORCE)

message(STATUS "PORIS_COMPONENTS_LIST (env)='${_raw_list}'")

# --- Deriva el slug de VARIANT para localizar build_<slug>/config.env -----
function(_poris_slug OUT VAR)
  string(TOLOWER "${VAR}" _low)
  string(REGEX REPLACE "[^a-z0-9]" "" _slug "${_low}")
  set(${OUT} "${_slug}" PARENT_SCOPE)
endfunction()

if(NOT DEFINED VARIANT AND DEFINED ENV{VARIANT})
  set(VARIANT "$ENV{VARIANT}")
endif()

if(DEFINED VARIANT AND NOT VARIANT STREQUAL "")
  _poris_slug(_PORIS_SLUG "${VARIANT}")
  set(_PORIS_CONFIG_ENV "${CMAKE_SOURCE_DIR}/build_${_PORIS_SLUG}/config.env")

  if(EXISTS "${_PORIS_CONFIG_ENV}")
    file(STRINGS "${_PORIS_CONFIG_ENV}" _PORIS_ENV_LINES
         REGEX "^[A-Za-z_][A-Za-z0-9_]*=.*")
    foreach(_line IN LISTS _PORIS_ENV_LINES)
      string(REGEX MATCH "^[A-Za-z_][A-Za-z0-9_]*" _k "${_line}")
      string(REGEX REPLACE "^[A-Za-z_][A-Za-z0-9_]*=" "" _v "${_line}")
      # Exporta al entorno del proceso CMake (visible a sub-steps como kconfiggen)
      set(ENV{${_k}} "${_v}")
      # Opcional: espejo CMake por si quieres consultarlo dentro del script
      set(PORIS_EXTRA_${_k} "${_v}" CACHE STRING "env from config.env" FORCE)
    endforeach()
    message(STATUS "PORIS: cargado entorno de ${_PORIS_CONFIG_ENV}")
  else()
    message(STATUS "PORIS: no hay ${_PORIS_CONFIG_ENV} (aún). Ejecuta prebuild primero si necesitas extra_env.")
  endif()
endif()


# --- PORIS: auto-cargar enables desde config.env del build dir --------------
# Deduce build dir (igual que el wrapper)
if(DEFINED VARIANT)
  string(TOLOWER "${VARIANT}" _var_suf)
  set(_poris_build_dir "${CMAKE_SOURCE_DIR}/build_${_var_suf}")
else()
  set(_poris_build_dir "${CMAKE_BINARY_DIR}")
endif()

# --- PORIS bootstrap para VSCode Configure ---------------------------------
# Si VSCode lanza "Configure" sin prebuild y aún no existe config.env,
# generamos la variante aquí para que Opción B tenga algo que cargar.
if(NOT DEFINED VARIANT AND DEFINED ENV{VARIANT})
  set(VARIANT "$ENV{VARIANT}")
endif()

if(DEFINED VARIANT)
  string(TOLOWER "${VARIANT}" _var_suf)
  set(_poris_build_dir "${CMAKE_SOURCE_DIR}/build_${_var_suf}")
else()
  # Último recurso: usa el binario actual
  set(_poris_build_dir "${CMAKE_BINARY_DIR}")
endif()

set(_poris_env_file "${_poris_build_dir}/config.env")
if(NOT EXISTS "${_poris_env_file}")
  message(STATUS "PORIS bootstrap: generando artefactos de variante para '${VARIANT}'")
  find_package(Python3 COMPONENTS Interpreter REQUIRED)

  execute_process(
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/gen_variant.py --variant ${VARIANT}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE _rv1
  )
  if(NOT _rv1 EQUAL 0)
    message(FATAL_ERROR "gen_variant.py falló con código ${_rv1}")
  endif()

  execute_process(
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/ensure_variant.py --variant ${VARIANT}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE _rv2
  )
  if(NOT _rv2 EQUAL 0)
    message(FATAL_ERROR "ensure_variant.py falló con código ${_rv2}")
  endif()
endif()
# ---------------------------------------------------------------------------


set(_poris_env_file "${_poris_build_dir}/config.env")
if(EXISTS "${_poris_env_file}")
  message(STATUS "PORIS: cargado entorno de ${_poris_env_file}")

  file(READ "${_poris_env_file}" _poris_env_txt)
  string(REPLACE "\r\n" "\n" _poris_env_txt "${_poris_env_txt}")
  separate_arguments(_poris_env_lines UNIX_COMMAND "${_poris_env_txt}")

  foreach(_line IN LISTS _poris_env_lines)
    if(_line MATCHES "^([A-Za-z0-9_]+)=(.*)$")
      set(_k "${CMAKE_MATCH_1}")
      set(_v "${CMAKE_MATCH_2}")
      # Exporta a ENV de este proceso CMake (por si algún script lo mira)
      set(ENV{${_k}} "${_v}")
      # Si es un PORIS_ENABLE_*, crea TAMBIÉN variable CMake (BOOL) para gating por CMake
      if(_k MATCHES "^PORIS_ENABLE_[A-Z0-9_]+$")
        # CMake trata "1", "ON", "TRUE" como verdadero
        set(${_k} ON CACHE BOOL "Enable ${_k} from config.env" FORCE)
      endif()
    endif()
  endforeach()
endif()
# ---------------------------------------------------------------------------


# --- IDF project.cmake + nombre de proyecto --------------------------------
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(poris_base)

# --- Mensajería útil post-config -------------------------------------------
message(STATUS "================ poris_base build context ================")
message(STATUS "  VARIANT            : ${VARIANT}")
message(STATUS "  IDF_TARGET         : ${IDF_TARGET}")
message(STATUS "  SDKCONFIG_DEFAULTS : ${SDKCONFIG_DEFAULTS}")
message(STATUS "=======================================================")
