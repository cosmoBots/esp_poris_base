#!/usr/bin/env bash
# tools/build.sh
set -euo pipefail

usage() {
  echo "uso: $0 <VariantId|any-case> [idf.py args...]" >&2
  echo "ej.:  $0 c6devkit build" >&2
  echo "      $0 C6Devkit flash monitor" >&2
  exit 2
}

[ $# -ge 1 ] || usage
REQ_VAR="$1"; shift || true

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VARS_YML="${ROOT}/variants/variants.yml"

if [ ! -f "$VARS_YML" ]; then
  echo "No encuentro ${VARS_YML}" >&2
  exit 3
fi

# Resuelve el ID canónico (case-insensitive) desde variants.yml
CANON_ID="$(python3 - <<'PY' "$REQ_VAR" "$VARS_YML"
import sys, yaml, re, pathlib
req = sys.argv[1].lower()
cfg = yaml.safe_load(pathlib.Path(sys.argv[2]).read_text(encoding="utf-8")) or {}
vs  = cfg.get("variants", [])
for v in vs:
    vid = (v.get("id") or "")
    if vid.lower() == req:
        print(vid); break
PY
)"
if [ -z "${CANON_ID:-}" ]; then
  echo "Variante '${REQ_VAR}' no encontrada en ${VARS_YML}" >&2
  exit 4
fi

# Slug para el directorio de build
SLUG="$(echo "$CANON_ID" | tr '[:upper:]' '[:lower:]' | sed -E 's/[^a-z0-9]+//g')"
BUILD_DIR="${ROOT}/build_${SLUG}"

# 1) Prebuild (genera defaults/env/components cacheados)
bash "${ROOT}/tools/prebuild.sh" "$CANON_ID"

# 2) Carga config.env si existe (extra_env para kconfig, etc.)
if [ -f "${BUILD_DIR}/config.env" ]; then
  set -a
  . "${BUILD_DIR}/config.env"
  set +a
fi

# 3) Llama a idf.py con el build dir y la variante; propaga args extra
exec idf.py -B "${BUILD_DIR}" -DVARIANT="${CANON_ID}" "$@"
