#!/usr/bin/env python3
# tools/new_component.py
import argparse, os, pathlib, re, shutil, sys

PLACEHOLDER = "$$1"

def env_key_for_component(name: str) -> str:
    # OtCoap -> PORIS_ENABLE_OTCOAP
    u = re.sub(r'[^A-Za-z0-9]', '_', name).upper()
    u = re.sub(r'_+', '_', u).strip('_')
    return f"PORIS_ENABLE_{u}"

def copy_template_tree(tmpl_dir: pathlib.Path, dest_dir: pathlib.Path, old: str, new: str):
    if dest_dir.exists():
        raise FileExistsError(f"El destino ya existe: {dest_dir}")
    for root, _, files in os.walk(tmpl_dir):
        rel = pathlib.Path(root).relative_to(tmpl_dir)
        rel_str = str(rel).replace(old, new)
        out_dir = dest_dir / rel_str if rel_str != '.' else dest_dir
        out_dir.mkdir(parents=True, exist_ok=True)
        for f in files:
            src = pathlib.Path(root) / f
            new_name = f.replace(old, new)
            dst = out_dir / new_name
            data = src.read_bytes()
            try:
                s = data.decode('utf-8').replace(old, new)
                dst.write_text(s, encoding='utf-8')
            except UnicodeDecodeError:
                shutil.copy2(src, dst)

def ensure_cmakelists_guard(comp_dir: pathlib.Path, comp_name: str, env_key: str):
    cmake = comp_dir / "CMakeLists.txt"
    if not cmake.exists():
        cmake.write_text(f"""# Auto-generated CMakeLists for {comp_name}
if(NOT DEFINED ENV{{{env_key}}})
  message(STATUS "[{comp_name}] disabled by variant ({env_key} not set)")
  return()
endif()

idf_component_register(
  SRCS "{comp_name}.c"
  INCLUDE_DIRS "include"
)
""", encoding="utf-8")
        return
    text = cmake.read_text(encoding="utf-8")
    if f"ENV{{{env_key}}}" not in text and "PORIS_ENABLE_" not in text:
        text = f"""# Guard por variante (no compilar si no está habilitado)
if(NOT DEFINED ENV{{{env_key}}})
  message(STATUS "[{comp_name}] disabled by variant ({env_key} not set)")
  return()
endif()

""" + text
        text = text.replace(PLACEHOLDER + ".c", f"{comp_name}.c")
        cmake.write_text(text, encoding="utf-8")

def main():
    ap = argparse.ArgumentParser(description="Instancia un componente desde tools/templates/$$1")
    ap.add_argument("--name", required=True, help="Nombre del componente (PascalCase, p.ej. OtCoap)")
    ap.add_argument("--template", default="tools/templates/$$1", help="Directorio plantilla con '$$1'")
    ap.add_argument("--dest", default="components", help="Directorio destino (components)")
    args = ap.parse_args()

    root = pathlib.Path(__file__).resolve().parent.parent
    tmpl_dir = (root / args.template).resolve()
    dest_parent = (root / args.dest).resolve()

    if not tmpl_dir.exists():
        print(f"No encuentro la plantilla: {tmpl_dir}", file=sys.stderr); sys.exit(2)
    if str(tmpl_dir).startswith(str(dest_parent)):
        print("La plantilla no debe estar dentro de 'components/'. Usa tools/templates/$$1.", file=sys.stderr); sys.exit(3)

    comp_name = args.name  # conservar PascalCase
    env_key = env_key_for_component(comp_name)

    dest_dir = dest_parent / comp_name
    copy_template_tree(tmpl_dir, dest_dir, PLACEHOLDER, comp_name)
    ensure_cmakelists_guard(dest_dir, comp_name, env_key)

    # Ajuste header si quedó como $$1.h
    inc = dest_dir / "include"
    old_h = inc / (PLACEHOLDER + ".h")
    if old_h.exists():
        old_h.rename(inc / f"{comp_name}.h")

    print(f"Componente creado en: {dest_dir}")
    print(f"Habilítalo en variantes con: {env_key}=1")

if __name__ == "__main__":
    main()
