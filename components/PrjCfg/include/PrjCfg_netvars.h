#pragma once
#ifndef PRJCFG_NETVARS_H
#define PRJCFG_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"
#include "nvs.h"
// Este include debe definir prjcfg_t y la instancia global prjcfg
#include "PrjCfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PRJCFG_NETVAR_TYPE_BOOL,
    PRJCFG_NETVAR_TYPE_U8,
    PRJCFG_NETVAR_TYPE_I8,
    PRJCFG_NETVAR_TYPE_I32,
    PRJCFG_NETVAR_TYPE_U32,
    PRJCFG_NETVAR_TYPE_STRING,
} PrjCfg_netvar_type_t;

typedef enum {
    PRJCFG_NVS_NONE,
    PRJCFG_NVS_LOAD,
    PRJCFG_NVS_SAVE,
    PRJCFG_NVS_LOADSAVE,
} PrjCfg_nvs_mode_t;

typedef struct {
    const char           *name;
    const char           *nvs_key;
    const char           *json_key;
    const char           *group;
    const char           *module;
    PrjCfg_netvar_type_t           type;
    PrjCfg_nvs_mode_t            nvs_mode;
    bool                  json;     // se expone en JSON
    void                 *ptr;      // &prjcfg.campo o prjcfg.array
} PrjCfg_netvar_desc_t;

extern const PrjCfg_netvar_desc_t PrjCfg_netvars_desc[];
extern const size_t      PrjCfg_netvars_count;

/* Funciones genéricas para este módulo (operan sobre la instancia global prjcfg) */
void PrjCfg_netvars_nvs_load(nvs_handle_t h);
void PrjCfg_netvars_nvs_save(nvs_handle_t h);

void PrjCfg_netvars_append_json(cJSON *root);
void PrjCfg_netvars_parse_json_dict(cJSON *root, bool *out_nvs_changed);

#ifdef __cplusplus
}
#endif

#endif // PRJCFG_NETVARS_H
