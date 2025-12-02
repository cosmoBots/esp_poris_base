#!/usr/bin/env python3
import csv
import sys
from pathlib import Path
import re


def sanitize_for_macro(name: str) -> str:
    s = re.sub(r"[^0-9A-Za-z]", "_", name)
    return s.upper()


def storage_to_enum(mod_upper: str, storage_type: str) -> str:
    storage_type = (storage_type or "").strip().upper()
    base = f"{mod_upper}_NETVAR_TYPE"
    mapping = {
        "BOOL":   f"{base}_BOOL",
        "U8":     f"{base}_U8",
        "I8":     f"{base}_I8",
        "I32":    f"{base}_I32",
        "U32":    f"{base}_U32",
        "STRING": f"{base}_STRING",
    }
    if storage_type not in mapping:
        raise ValueError(f"Unknown storage_type '{storage_type}'")
    return mapping[storage_type]


def nvs_mode_to_enum(mod_upper: str, mode: str) -> str:
    mode = (mode or "").strip().upper()
    base = f"{mod_upper}_NVS"
    mapping = {
        "NONE":     f"{base}_NONE",
        "LOAD":     f"{base}_LOAD",
        "SAVE":     f"{base}_SAVE",
        "LOADSAVE": f"{base}_LOADSAVE",
        "":         f"{base}_NONE",
    }
    if mode not in mapping:
        raise ValueError(f"Unknown nvs_mode '{mode}'")
    return mapping[mode]


def read_rows(csv_path: Path):
    with csv_path.open(newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        return list(reader)


# ---------- fragmento en include/ ----------

def generate_fragment(mod: str, rows, out_path: Path):
    mod_lower = mod.lower()
    data_struct_type = f"{mod_lower}_t"

    lines = []
    lines.append(f"// Auto-generated fragment for {mod} netvars")
    lines.append(f"// Include this inside the definition of struct {data_struct_type}")
    lines.append("// DO NOT EDIT MANUALLY; edit netvars.csv and regenerate.")
    lines.append("")

    for r in rows:
        name = (r.get("name") or "").strip()
        c_type = (r.get("c_type") or "").strip()
        if not name or not c_type:
            continue

        # Detectar arrays: char[LEN] o char [LEN]
        m = re.match(r"^(.*?)(\s*\[.*\])$", c_type)
        if m:
            base = m.group(1).strip()
            array_suffix = m.group(2)
            lines.append(f"    {base} {name}{array_suffix};")
        else:
            lines.append(f"    {c_type} {name};")

    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Generated fragment {out_path}")


# ---------- header público en components/PrjCfg/ ----------

def generate_public_header(mod: str, rows, out_path: Path):
    mod_upper = sanitize_for_macro(mod)
    guard = f"{mod_upper}_NETVARS_H"

    mod_lower = mod.lower()
    data_struct_type = f"{mod_lower}_t"
    data_instance = mod_lower

    type_enum = f"{mod}_netvar_type_t"
    nvs_enum = f"{mod}_nvs_mode_t"
    desc_type = f"{mod}_netvar_desc_t"
    desc_array_name = f"{mod}_netvars_desc"
    count_name = f"{mod}_netvars_count"

    text = f"""#pragma once
#ifndef {guard}
#define {guard}

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"
#include "nvs.h"
// Este include debe definir {data_struct_type} y la instancia global {data_instance}
#include "{mod}.h"

#ifdef __cplusplus
extern "C" {{
#endif

typedef enum {{
    {mod_upper}_NETVAR_TYPE_BOOL,
    {mod_upper}_NETVAR_TYPE_U8,
    {mod_upper}_NETVAR_TYPE_I8,
    {mod_upper}_NETVAR_TYPE_I32,
    {mod_upper}_NETVAR_TYPE_U32,
    {mod_upper}_NETVAR_TYPE_STRING,
}} {type_enum};

typedef enum {{
    {mod_upper}_NVS_NONE,
    {mod_upper}_NVS_LOAD,
    {mod_upper}_NVS_SAVE,
    {mod_upper}_NVS_LOADSAVE,
}} {nvs_enum};

typedef struct {{
    const char           *name;
    const char           *nvs_key;
    const char           *json_key;
    const char           *group;
    const char           *module;
    {type_enum}           type;
    {nvs_enum}            nvs_mode;
    bool                  json;     // se expone en JSON
    void                 *ptr;      // &{data_instance}.campo o {data_instance}.array
}} {desc_type};

extern const {desc_type} {desc_array_name}[];
extern const size_t      {count_name};

/* Funciones genéricas para este módulo (operan sobre la instancia global {data_instance}) */
void {mod}_netvars_nvs_load(nvs_handle_t h);
void {mod}_netvars_nvs_save(nvs_handle_t h);

void {mod}_netvars_append_json(cJSON *root);
void {mod}_netvars_parse_json_dict(cJSON *root, bool *out_nvs_changed);

#ifdef __cplusplus
}}
#endif

#endif // {guard}
"""

    out_path.write_text(text, encoding="utf-8")
    print(f"Generated header {out_path}")


# ---------- source en components/PrjCfg/ ----------

def generate_source(mod: str, rows, out_path: Path):
    mod_upper = sanitize_for_macro(mod)
    mod_lower = mod.lower()
    data_struct_type = f"{mod_lower}_t"
    data_instance = mod_lower

    type_enum = f"{mod}_netvar_type_t"
    nvs_enum = f"{mod}_nvs_mode_t"
    desc_type = f"{mod}_netvar_desc_t"
    desc_array_name = f"{mod}_netvars_desc"
    count_name = f"{mod}_netvars_count"

    entries = []
    for r in rows:
        name = (r.get("name") or "").strip()
        if not name:
            continue

        storage_type = (r.get("storage_type") or "").strip()
        nvs_key = (r.get("nvs_key") or "").strip()
        json_key = (r.get("json_key") or "").strip()
        group = (r.get("group") or "").strip()
        module = (r.get("module") or "").strip()
        nvs_mode = (r.get("nvs_mode") or "").strip()
        json_flag = (r.get("json") or "").strip()

        type_enum_val = storage_to_enum(mod_upper, storage_type)
        nvs_enum_val = nvs_mode_to_enum(mod_upper, nvs_mode)

        nvs_c    = "NULL" if not nvs_key else f"\"{nvs_key}\""
        json_c   = "NULL" if not json_key else f"\"{json_key}\""
        group_c  = "NULL" if not group else f"\"{group}\""
        module_c = "NULL" if not module else f"\"{module}\""

        json_bool = "true" if json_flag == "1" else "false"

        if storage_type.upper() == "STRING":
            ptr_expr = f"(void*){data_instance}.{name}"
        else:
            ptr_expr = f"(void*)&{data_instance}.{name}"

        entry = (
            f'    {{ "{name}", {nvs_c}, {json_c}, {group_c}, {module_c}, '
            f'{type_enum_val}, {nvs_enum_val}, {json_bool}, {ptr_expr} }},'
        )
        entries.append(entry)

    entries_text = "\n".join(entries)

    text = f"""#include "{mod}_netvars.h"
#include <string.h>

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// {data_struct_type} {data_instance};
extern {data_struct_type} {data_instance};

const {desc_type} {desc_array_name}[] = {{
{entries_text}
}};

const size_t {count_name} = sizeof({desc_array_name}) / sizeof({desc_array_name}[0]);

void {mod}_netvars_nvs_load(nvs_handle_t h)
{{
    esp_err_t err;

    for (size_t i = 0; i < {count_name}; ++i) {{
        const {desc_type} *d = &{desc_array_name}[i];
        if (!d->nvs_key) continue;
        if (d->nvs_mode != {mod_upper}_NVS_LOAD &&
            d->nvs_mode != {mod_upper}_NVS_LOADSAVE) continue;

        switch (d->type) {{
        case {mod_upper}_NETVAR_TYPE_BOOL: {{
            uint8_t v;
            err = nvs_get_u8(h, d->nvs_key, &v);
            if (err == ESP_OK) *(bool *)d->ptr = (bool)v;
            break;
        }}
        case {mod_upper}_NETVAR_TYPE_U8: {{
            uint8_t v;
            err = nvs_get_u8(h, d->nvs_key, &v);
            if (err == ESP_OK) *(uint8_t *)d->ptr = v;
            break;
        }}
        case {mod_upper}_NETVAR_TYPE_I8: {{
            int8_t v;
            err = nvs_get_i8(h, d->nvs_key, &v);
            if (err == ESP_OK) *(int8_t *)d->ptr = v;
            break;
        }}
        case {mod_upper}_NETVAR_TYPE_I32: {{
            int32_t v;
            err = nvs_get_i32(h, d->nvs_key, &v);
            if (err == ESP_OK) *(int32_t *)d->ptr = v;
            break;
        }}
        case {mod_upper}_NETVAR_TYPE_U32: {{
            uint32_t v;
            err = nvs_get_u32(h, d->nvs_key, &v);
            if (err == ESP_OK) *(uint32_t *)d->ptr = v;
            break;
        }}
        case {mod_upper}_NETVAR_TYPE_STRING:
            break;
        }}
    }}
}}

void {mod}_netvars_nvs_save(nvs_handle_t h)
{{
    esp_err_t err;

    for (size_t i = 0; i < {count_name}; ++i) {{
        const {desc_type} *d = &{desc_array_name}[i];
        if (!d->nvs_key) continue;
        if (d->nvs_mode != {mod_upper}_NVS_SAVE &&
            d->nvs_mode != {mod_upper}_NVS_LOADSAVE) continue;

        switch (d->type) {{
        case {mod_upper}_NETVAR_TYPE_BOOL:
            err = nvs_set_u8(h, d->nvs_key, *(bool *)d->ptr ? 1 : 0);
            break;
        case {mod_upper}_NETVAR_TYPE_U8:
            err = nvs_set_u8(h, d->nvs_key, *(uint8_t *)d->ptr);
            break;
        case {mod_upper}_NETVAR_TYPE_I8:
            err = nvs_set_i8(h, d->nvs_key, *(int8_t *)d->ptr);
            break;
        case {mod_upper}_NETVAR_TYPE_I32:
            err = nvs_set_i32(h, d->nvs_key, *(int32_t *)d->ptr);
            break;
        case {mod_upper}_NETVAR_TYPE_U32:
            err = nvs_set_u32(h, d->nvs_key, *(uint32_t *)d->ptr);
            break;
        case {mod_upper}_NETVAR_TYPE_STRING:
            err = ESP_OK;
            break;
        }}

        if (err != ESP_OK) {{
            // opcional: logs
        }}
    }}

    (void)nvs_commit(h);
}}

void {mod}_netvars_append_json(cJSON *root)
{{
    for (size_t i = 0; i < {count_name}; ++i) {{
        const {desc_type} *d = &{desc_array_name}[i];
        if (!d->json || !d->json_key) continue;

        switch (d->type) {{
        case {mod_upper}_NETVAR_TYPE_BOOL:
            cJSON_AddBoolToObject(root, d->json_key, *(bool *)d->ptr);
            break;
        case {mod_upper}_NETVAR_TYPE_U8:
            cJSON_AddNumberToObject(root, d->json_key, *(uint8_t *)d->ptr);
            break;
        case {mod_upper}_NETVAR_TYPE_I8:
            cJSON_AddNumberToObject(root, d->json_key, *(int8_t *)d->ptr);
            break;
        case {mod_upper}_NETVAR_TYPE_I32:
            cJSON_AddNumberToObject(root, d->json_key, *(int32_t *)d->ptr);
            break;
        case {mod_upper}_NETVAR_TYPE_U32:
            cJSON_AddNumberToObject(root, d->json_key, *(uint32_t *)d->ptr);
            break;
        case {mod_upper}_NETVAR_TYPE_STRING:
            cJSON_AddStringToObject(root, d->json_key, (const char *)d->ptr);
            break;
        }}
    }}
}}

static const {desc_type} *
{mod}_netvars_find_by_json_key(const char *key)
{{
    for (size_t i = 0; i < {count_name}; ++i) {{
        const {desc_type} *d = &{desc_array_name}[i];
        if (d->json_key && strcmp(d->json_key, key) == 0)
            return d;
    }}
    return NULL;
}}

void {mod}_netvars_parse_json_dict(cJSON *root, bool *out_nvs_changed)
{{
    if (out_nvs_changed) *out_nvs_changed = false;

    cJSON *nvi = NULL;
    cJSON_ArrayForEach(nvi, root) {{
        const {desc_type} *d =
            {mod}_netvars_find_by_json_key(nvi->string);
        if (!d) continue;

        switch (d->type) {{
        case {mod_upper}_NETVAR_TYPE_BOOL: {{
            bool value = cJSON_IsTrue(nvi);
            if (value != *(bool *)d->ptr) {{
                *(bool *)d->ptr = value;
                if (out_nvs_changed) *out_nvs_changed = true;
            }}
            break;
        }}
        case {mod_upper}_NETVAR_TYPE_U8: {{
            uint8_t value = (uint8_t)nvi->valueint;
            if (value != *(uint8_t *)d->ptr) {{
                *(uint8_t *)d->ptr = value;
                if (out_nvs_changed) *out_nvs_changed = true;
            }}
            break;
        }}
        case {mod_upper}_NETVAR_TYPE_I8: {{
            int8_t value = (int8_t)nvi->valueint;
            if (value != *(int8_t *)d->ptr) {{
                *(int8_t *)d->ptr = value;
                if (out_nvs_changed) *out_nvs_changed = true;
            }}
            break;
        }}
        case {mod_upper}_NETVAR_TYPE_I32: {{
            int32_t value = (int32_t)nvi->valueint;
            if (value != *(int32_t *)d->ptr) {{
                *(int32_t *)d->ptr = value;
                if (out_nvs_changed) *out_nvs_changed = true;
            }}
            break;
        }}
        case {mod_upper}_NETVAR_TYPE_U32: {{
            uint32_t value = (uint32_t)nvi->valueint;
            if (value != *(uint32_t *)d->ptr) {{
                *(uint32_t *)d->ptr = value;
                if (out_nvs_changed) *out_nvs_changed = true;
            }}
            break;
        }}
        case {mod_upper}_NETVAR_TYPE_STRING:
            // TODO: strncpy con límite si quieres que JSON actualice strings
            break;
        }}
    }}
}}
"""

    out_path.write_text(text, encoding="utf-8")
    print(f"Generated source {out_path}")


def main():
    if len(sys.argv) != 2:
        print("Usage: gen_netvars.py <ModuleName>")
        print("Example: gen_netvars.py PrjCfg")
        sys.exit(1)

    mod = sys.argv[1]  # "PrjCfg", "Thermostat", etc.

    # Script en poris/, repo root = padre del padre
    root_dir = Path(__file__).resolve().parent.parent
    comp_dir = root_dir / "components" / mod

    csv_path = comp_dir / "netvars.csv"
    if not csv_path.exists():
        print(f"ERROR: CSV file '{csv_path}' not found")
        sys.exit(1)

    rows = read_rows(csv_path)

    include_dir = comp_dir / "include"
    include_dir.mkdir(parents=True, exist_ok=True)

    fragment_path = include_dir / f"{mod}_netvar_types_fragment.h_"
    pub_h_path = comp_dir / f"{mod}_netvars.h"
    c_path = comp_dir / f"{mod}_netvars.c"

    generate_fragment(mod, rows, fragment_path)
    generate_public_header(mod, rows, pub_h_path)
    generate_source(mod, rows, c_path)


if __name__ == "__main__":
    main()
