
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#include "cJSON.h"
#include <nvs.h>

#include <NetVars.h>

void NetVars_nvs_load(const NetVars_desc_t netvars_desc[], const size_t netvars_count, nvs_handle_t h)
{
    esp_err_t err;

    for (size_t i = 0; i < netvars_count; ++i) {
        const NetVars_desc_t *d = &netvars_desc[i];
        if (!d->nvs_key) continue;
        if (d->nvs_mode != PRJCFG_NVS_LOAD &&
            d->nvs_mode != PRJCFG_NVS_LOADSAVE) continue;

        switch (d->type) {
        case NETVARS_TYPE_BOOL: {
            uint8_t v;
            err = nvs_get_u8(h, d->nvs_key, &v);
            if (err == ESP_OK) *(bool *)d->ptr = (bool)v;
            break;
        }
        case NETVARS_TYPE_U8: {
            uint8_t v;
            err = nvs_get_u8(h, d->nvs_key, &v);
            if (err == ESP_OK) *(uint8_t *)d->ptr = v;
            break;
        }
        case NETVARS_TYPE_I8: {
            int8_t v;
            err = nvs_get_i8(h, d->nvs_key, &v);
            if (err == ESP_OK) *(int8_t *)d->ptr = v;
            break;
        }
        case NETVARS_TYPE_I32: {
            int32_t v;
            err = nvs_get_i32(h, d->nvs_key, &v);
            if (err == ESP_OK) *(int32_t *)d->ptr = v;
            break;
        }
        case NETVARS_TYPE_U32: {
            uint32_t v;
            err = nvs_get_u32(h, d->nvs_key, &v);
            if (err == ESP_OK) *(uint32_t *)d->ptr = v;
            break;
        }
        case NETVARS_TYPE_STRING:
            break;
        }
    }
}

void NetVars_nvs_save(const NetVars_desc_t netvars_desc[], const size_t netvars_count, nvs_handle_t h)
{
    esp_err_t err;

    for (size_t i = 0; i < netvars_count; ++i) {
        const NetVars_desc_t *d = &netvars_desc[i];
        if (!d->nvs_key) continue;
        if (d->nvs_mode != PRJCFG_NVS_SAVE &&
            d->nvs_mode != PRJCFG_NVS_LOADSAVE) continue;

        switch (d->type) {
        case NETVARS_TYPE_BOOL:
            err = nvs_set_u8(h, d->nvs_key, *(bool *)d->ptr ? 1 : 0);
            break;
        case NETVARS_TYPE_U8:
            err = nvs_set_u8(h, d->nvs_key, *(uint8_t *)d->ptr);
            break;
        case NETVARS_TYPE_I8:
            err = nvs_set_i8(h, d->nvs_key, *(int8_t *)d->ptr);
            break;
        case NETVARS_TYPE_I32:
            err = nvs_set_i32(h, d->nvs_key, *(int32_t *)d->ptr);
            break;
        case NETVARS_TYPE_U32:
            err = nvs_set_u32(h, d->nvs_key, *(uint32_t *)d->ptr);
            break;
        case NETVARS_TYPE_STRING:
            err = ESP_OK;
            break;
        }

        if (err != ESP_OK) {
            // opcional: logs
        }
    }

    (void)nvs_commit(h);
}

void NetVars_append_json(const NetVars_desc_t netvars_desc[], const size_t netvars_count, cJSON *root)
{
    for (size_t i = 0; i < netvars_count; ++i) {
        const NetVars_desc_t *d = &netvars_desc[i];
        if (!d->json || !d->json_key) continue;

        switch (d->type) {
        case NETVARS_TYPE_BOOL:
            cJSON_AddBoolToObject(root, d->json_key, *(bool *)d->ptr);
            break;
        case NETVARS_TYPE_U8:
            cJSON_AddNumberToObject(root, d->json_key, *(uint8_t *)d->ptr);
            break;
        case NETVARS_TYPE_I8:
            cJSON_AddNumberToObject(root, d->json_key, *(int8_t *)d->ptr);
            break;
        case NETVARS_TYPE_I32:
            cJSON_AddNumberToObject(root, d->json_key, *(int32_t *)d->ptr);
            break;
        case NETVARS_TYPE_U32:
            cJSON_AddNumberToObject(root, d->json_key, *(uint32_t *)d->ptr);
            break;
        case NETVARS_TYPE_STRING:
            cJSON_AddStringToObject(root, d->json_key, (const char *)d->ptr);
            break;
        }
    }
}

static const NetVars_desc_t *
NetVars_find_by_json_key(const NetVars_desc_t netvars_desc[], const size_t netvars_count, const char *key)
{
    for (size_t i = 0; i < netvars_count; ++i) {
        const NetVars_desc_t *d = &netvars_desc[i];
        if (d->json_key && strcmp(d->json_key, key) == 0)
            return d;
    }
    return NULL;
}

bool NetVars_parse_json_dict(const NetVars_desc_t netvars_desc[], const size_t netvars_count, cJSON *root)
{
    bool out_nvs_changed = false;

    cJSON *nvi = NULL;
    cJSON_ArrayForEach(nvi, root) {
        const NetVars_desc_t *d =
            NetVars_find_by_json_key(netvars_desc, netvars_count, nvi->string);
        if (!d) continue;

        switch (d->type) {
        case NETVARS_TYPE_BOOL: {
            bool value = cJSON_IsTrue(nvi);
            if (value != *(bool *)d->ptr) {
                *(bool *)d->ptr = value;
                out_nvs_changed = true;
            }
            break;
        }
        case NETVARS_TYPE_U8: {
            uint8_t value = (uint8_t)nvi->valueint;
            if (value != *(uint8_t *)d->ptr) {
                *(uint8_t *)d->ptr = value;
                out_nvs_changed = true;
            }
            break;
        }
        case NETVARS_TYPE_I8: {
            int8_t value = (int8_t)nvi->valueint;
            if (value != *(int8_t *)d->ptr) {
                *(int8_t *)d->ptr = value;
                out_nvs_changed = true;
            }
            break;
        }
        case NETVARS_TYPE_I32: {
            int32_t value = (int32_t)nvi->valueint;
            if (value != *(int32_t *)d->ptr) {
                *(int32_t *)d->ptr = value;
                out_nvs_changed = true;
            }
            break;
        }
        case NETVARS_TYPE_U32: {
            uint32_t value = (uint32_t)nvi->valueint;
            if (value != *(uint32_t *)d->ptr) {
                *(uint32_t *)d->ptr = value;
                out_nvs_changed = true;
            }
            break;
        }
        case NETVARS_TYPE_STRING:
            // TODO: strncpy con límite si quieres que JSON actualice strings
            break;
        }
    }
    return out_nvs_changed;
}
