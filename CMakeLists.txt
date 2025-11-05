cmake_minimum_required(VERSION 3.16)

# --- Entrada de usuario (opcional por CLI o variables de entorno) ---
#   -DVARIANT=NombreDeVariante   (p.ej., S3LCDWifi)
#   -DIDF_TARGET=esp32s3         (normalmente NO hace falta si usas gen_variant.py)
if(NOT DEFINED VARIANT AND DEFINED ENV{VARIANT})
  set(VARIANT "$ENV{VARIANT}")
endif()

# Ruta raíz y utilidades
set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")
set(BUILDCFG_DIR  "${PROJECT_ROOT}/buildcfg")

# --- Cargar cache de la variante (si se ha pasado VARIANT) ---
if(VARIANT)
  set(VAR_CACHE    "${BUILDCFG_DIR}/${VARIANT}.cmakecache.cmake")
  set(VAR_SDKDEFS  "${BUILDCFG_DIR}/sdkconfig.${VARIANT}.defaults")

  if(EXISTS "${VAR_CACHE}")
    message(STATUS "Incluyendo cache de variante: ${VAR_CACHE}")
    include("${VAR_CACHE}")  # esto hace set(IDF_TARGET ...)
  else()
    message(FATAL_ERROR
      "No existe ${VAR_CACHE}.\n"
      "Genera la variante primero: python3 tools/gen_variant.py --variant ${VARIANT}")
  endif()

  if(EXISTS "${VAR_SDKDEFS}")
    # IMPORTANTE: IDF acepta lista separada por ';' en SDKCONFIG_DEFAULTS; aquí sólo uno.
    set(SDKCONFIG_DEFAULTS "${VAR_SDKDEFS}" CACHE STRING "sdkconfig defaults for variant" FORCE)
    message(STATUS "Usando SDKCONFIG_DEFAULTS=${SDKCONFIG_DEFAULTS}")
  else()
    message(FATAL_ERROR
      "No existe ${VAR_SDKDEFS}.\n"
      "Genera la variante: python3 tools/gen_variant.py --variant ${VARIANT}")
  endif()
else()
  message(STATUS "VARIANT no establecido. Usaré sdkconfig.defaults (si existe).")
  # si no hay variante, dejamos que IDF use sdkconfig.defaults por defecto.
endif()

# --- IDF target ---
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

# --- IDF project.cmake + nombre de proyecto ---
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(poris_base)

# --- Mensajería útil post-config ---
message(STATUS "================ poris_base build context ================")
message(STATUS "  VARIANT            : ${VARIANT}")
message(STATUS "  IDF_TARGET         : ${IDF_TARGET}")
message(STATUS "  SDKCONFIG_DEFAULTS : ${SDKCONFIG_DEFAULTS}")
message(STATUS "=======================================================")
