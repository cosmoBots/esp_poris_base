#!/usr/bin/env python3
# tools/gen_variant.py
import argparse, yaml, pathlib, sys

def concat_files(paths):
    out = []
    for p in paths:
        pp = pathlib.Path(p)
        if not pp.exists():
            raise FileNotFoundError(str(pp))
        out.append(pp.read_text(encoding="utf-8").rstrip() + "\n")
    return "".join(out)

def ensure_file(path: pathlib.Path, on_create_comment: str):
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists():
        path.write_text(f"# {on_create_comment}\n", encoding="utf-8")

def main():
    ap = argparse.ArgumentParser(description="Genera sdkconfig.<variant>.defaults a partir de variants.yml")
    ap.add_argument("--variant", required=True, help="ID de la variante definida en variants/variants.yml")
    ap.add_argument("--variants-yml", default="variants/variants.yml")
    args = ap.parse_args()

    # Raíz del proyecto (asumiendo este script dentro de tools/)
    script_dir = pathlib.Path(__file__).resolve().parent
    root = script_dir.parent

    variants_yml = (root / args.variants_yml).resolve()
    if not variants_yml.exists():
        print(f"No encuentro {variants_yml}", file=sys.stderr)
        sys.exit(2)

    cfg = yaml.safe_load(variants_yml.read_text(encoding="utf-8"))
    var = next((v for v in cfg.get("variants", []) if v.get("id") == args.variant), None)
    if not var:
        print(f"Variante '{args.variant}' no definida en {variants_yml}", file=sys.stderr)
        sys.exit(3)

    # Directorios desde YAML (con defaults sensatos)
    defaults_dir = (root / cfg.get("defaults_dir", "sdkconfig")).resolve()
    out_dir = (root / cfg.get("out_dir", "buildcfg")).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    # sdkconfig.defaults base (no decide nada: vacío si no existe)
    base_defaults = root / "sdkconfig.defaults"
    ensure_file(base_defaults, "base defaults (auto-created empty)")

    # Asegurar overlays: si faltan, los creamos VACÍOS (solo comentario)
    overlays = list(var.get("overlays", []))
    inputs = [str(base_defaults)]
    for ov in overlays:
        ov_path = defaults_dir / ov
        ensure_file(ov_path, f"auto-created empty overlay: {ov}")
        inputs.append(str(ov_path))

    # Combinar
    content = concat_files(inputs)
    out_path = out_dir / f"sdkconfig.{var['id']}.defaults"
    out_path.write_text(content, encoding="utf-8")

    # Hint CMake con el target que viene en YAML (si existe)
    cache = out_dir / f"{var['id']}.cmakecache.cmake"
    target = var.get("target")  # opcional, pero recomendado en tu YAML
    cache.write_text(f"set(IDF_TARGET {target})\n" if target else "# no target in YAML\n", encoding="utf-8")

    print(str(out_path))

if __name__ == "__main__":
    main()
