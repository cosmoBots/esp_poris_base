#include "PrjCfg_netvars.h"
#include <string.h>

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// prjcfg_t PrjCfg_dre;
extern PrjCfg_dre_t PrjCfg_dre;

const PrjCfg_netvar_desc_t PrjCfg_netvars_desc[] = {
    { "central_role", "rcentr", "rcentr", "config", "roles", PRJCFG_NETVAR_TYPE_BOOL, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.central_role },
    { "peripheral_role", "rperiph", "rperiph", "config", "roles", PRJCFG_NETVAR_TYPE_BOOL, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.peripheral_role },
    { "echo", "echo", "echo", "config", "uart", PRJCFG_NETVAR_TYPE_BOOL, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.echo },
    { "skip_ota", "otaskip", "otaskip", "config", "ota", PRJCFG_NETVAR_TYPE_BOOL, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.skip_ota },
    { "uart_bridge", "uartbr", "uartbr", "config", "uart", PRJCFG_NETVAR_TYPE_BOOL, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_bridge },
    { "ip_address", NULL, "ip", "config", "net", PRJCFG_NETVAR_TYPE_STRING, PRJCFG_NVS_NONE, true, (void*)PrjCfg_dre.ip_address },
    { "uart_user_port", "uuport", "uuport", "config", "uart_user", PRJCFG_NETVAR_TYPE_U8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_user_port },
    { "uart_user_baudrate", "uubaud", "uubaud", "config", "uart_user", PRJCFG_NETVAR_TYPE_I32, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_user_baudrate },
    { "uart_user_stop_bits", "uustopb", "uustopb", "config", "uart_user", PRJCFG_NETVAR_TYPE_U8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_user_stop_bits },
    { "uart_user_tx_io_num", "uutx", "uutx", "config", "uart_user", PRJCFG_NETVAR_TYPE_I8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_user_tx_io_num },
    { "uart_user_rx_io_num", "uurx", "uurx", "config", "uart_user", PRJCFG_NETVAR_TYPE_I8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_user_rx_io_num },
    { "uart_periph_port", "upport", "upport", "config", "uart_periph", PRJCFG_NETVAR_TYPE_U8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_periph_port },
    { "uart_periph_baudrate", "upbaud", "upbaud", "config", "uart_periph", PRJCFG_NETVAR_TYPE_I32, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_periph_baudrate },
    { "uart_periph_stop_bits", "upstopb", "upstopb", "config", "uart_periph", PRJCFG_NETVAR_TYPE_U8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_periph_stop_bits },
    { "uart_periph_tx_io_num", "uptx", "uptx", "config", "uart_periph", PRJCFG_NETVAR_TYPE_I8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_periph_tx_io_num },
    { "uart_periph_rx_io_num", "uprx", "uprx", "config", "uart_periph", PRJCFG_NETVAR_TYPE_I8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_periph_rx_io_num },
    { "blink_enabled", "blink", "blink", "config", "blink", PRJCFG_NETVAR_TYPE_BOOL, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.blink_enabled },
    { "blink_io_num", "blinkpin", "blinkpin", "config", "blink", PRJCFG_NETVAR_TYPE_I8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.blink_io_num },
    { "blink_on_value", "blinkval", "blinkval", "config", "blink", PRJCFG_NETVAR_TYPE_BOOL, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.blink_on_value },
    { "blink_on_ms", "blinkon", "blinkon", "config", "blink", PRJCFG_NETVAR_TYPE_U32, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.blink_on_ms },
    { "blink_off_ms", "blinkoff", "blinkoff", "config", "blink", PRJCFG_NETVAR_TYPE_U32, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.blink_off_ms },
    { "hover_enabled", "hover", "hover", "config", "hover", PRJCFG_NETVAR_TYPE_BOOL, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.hover_enabled },
    { "uart_hover_port", "uhport", "uhport", "config", "hover", PRJCFG_NETVAR_TYPE_U8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_hover_port },
    { "uart_hover_rx_io_num", "uhrx", "uhrx", "config", "hover", PRJCFG_NETVAR_TYPE_I8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_hover_rx_io_num },
    { "uart_hover_tx_io_num", "uhtx", "uhtx", "config", "hover", PRJCFG_NETVAR_TYPE_I8, PRJCFG_NVS_LOADSAVE, true, (void*)&PrjCfg_dre.uart_hover_tx_io_num },
};

const size_t PrjCfg_netvars_count = sizeof(PrjCfg_netvars_desc) / sizeof(PrjCfg_netvars_desc[0]);

void PrjCfg_netvars_nvs_load(nvs_handle_t h)
{
    esp_err_t err;

    for (size_t i = 0; i < PrjCfg_netvars_count; ++i) {
        const PrjCfg_netvar_desc_t *d = &PrjCfg_netvars_desc[i];
        if (!d->nvs_key) continue;
        if (d->nvs_mode != PRJCFG_NVS_LOAD &&
            d->nvs_mode != PRJCFG_NVS_LOADSAVE) continue;

        switch (d->type) {
        case PRJCFG_NETVAR_TYPE_BOOL: {
            uint8_t v;
            err = nvs_get_u8(h, d->nvs_key, &v);
            if (err == ESP_OK) *(bool *)d->ptr = (bool)v;
            break;
        }
        case PRJCFG_NETVAR_TYPE_U8: {
            uint8_t v;
            err = nvs_get_u8(h, d->nvs_key, &v);
            if (err == ESP_OK) *(uint8_t *)d->ptr = v;
            break;
        }
        case PRJCFG_NETVAR_TYPE_I8: {
            int8_t v;
            err = nvs_get_i8(h, d->nvs_key, &v);
            if (err == ESP_OK) *(int8_t *)d->ptr = v;
            break;
        }
        case PRJCFG_NETVAR_TYPE_I32: {
            int32_t v;
            err = nvs_get_i32(h, d->nvs_key, &v);
            if (err == ESP_OK) *(int32_t *)d->ptr = v;
            break;
        }
        case PRJCFG_NETVAR_TYPE_U32: {
            uint32_t v;
            err = nvs_get_u32(h, d->nvs_key, &v);
            if (err == ESP_OK) *(uint32_t *)d->ptr = v;
            break;
        }
        case PRJCFG_NETVAR_TYPE_STRING:
            break;
        }
    }
}

void PrjCfg_netvars_nvs_save(nvs_handle_t h)
{
    esp_err_t err;

    for (size_t i = 0; i < PrjCfg_netvars_count; ++i) {
        const PrjCfg_netvar_desc_t *d = &PrjCfg_netvars_desc[i];
        if (!d->nvs_key) continue;
        if (d->nvs_mode != PRJCFG_NVS_SAVE &&
            d->nvs_mode != PRJCFG_NVS_LOADSAVE) continue;

        switch (d->type) {
        case PRJCFG_NETVAR_TYPE_BOOL:
            err = nvs_set_u8(h, d->nvs_key, *(bool *)d->ptr ? 1 : 0);
            break;
        case PRJCFG_NETVAR_TYPE_U8:
            err = nvs_set_u8(h, d->nvs_key, *(uint8_t *)d->ptr);
            break;
        case PRJCFG_NETVAR_TYPE_I8:
            err = nvs_set_i8(h, d->nvs_key, *(int8_t *)d->ptr);
            break;
        case PRJCFG_NETVAR_TYPE_I32:
            err = nvs_set_i32(h, d->nvs_key, *(int32_t *)d->ptr);
            break;
        case PRJCFG_NETVAR_TYPE_U32:
            err = nvs_set_u32(h, d->nvs_key, *(uint32_t *)d->ptr);
            break;
        case PRJCFG_NETVAR_TYPE_STRING:
            err = ESP_OK;
            break;
        }

        if (err != ESP_OK) {
            // opcional: logs
        }
    }

    (void)nvs_commit(h);
}

void PrjCfg_netvars_append_json(cJSON *root)
{
    for (size_t i = 0; i < PrjCfg_netvars_count; ++i) {
        const PrjCfg_netvar_desc_t *d = &PrjCfg_netvars_desc[i];
        if (!d->json || !d->json_key) continue;

        switch (d->type) {
        case PRJCFG_NETVAR_TYPE_BOOL:
            cJSON_AddBoolToObject(root, d->json_key, *(bool *)d->ptr);
            break;
        case PRJCFG_NETVAR_TYPE_U8:
            cJSON_AddNumberToObject(root, d->json_key, *(uint8_t *)d->ptr);
            break;
        case PRJCFG_NETVAR_TYPE_I8:
            cJSON_AddNumberToObject(root, d->json_key, *(int8_t *)d->ptr);
            break;
        case PRJCFG_NETVAR_TYPE_I32:
            cJSON_AddNumberToObject(root, d->json_key, *(int32_t *)d->ptr);
            break;
        case PRJCFG_NETVAR_TYPE_U32:
            cJSON_AddNumberToObject(root, d->json_key, *(uint32_t *)d->ptr);
            break;
        case PRJCFG_NETVAR_TYPE_STRING:
            cJSON_AddStringToObject(root, d->json_key, (const char *)d->ptr);
            break;
        }
    }
}

static const PrjCfg_netvar_desc_t *
PrjCfg_netvars_find_by_json_key(const char *key)
{
    for (size_t i = 0; i < PrjCfg_netvars_count; ++i) {
        const PrjCfg_netvar_desc_t *d = &PrjCfg_netvars_desc[i];
        if (d->json_key && strcmp(d->json_key, key) == 0)
            return d;
    }
    return NULL;
}

void PrjCfg_netvars_parse_json_dict(cJSON *root, bool *out_nvs_changed)
{
    if (out_nvs_changed) *out_nvs_changed = false;

    cJSON *nvi = NULL;
    cJSON_ArrayForEach(nvi, root) {
        const PrjCfg_netvar_desc_t *d =
            PrjCfg_netvars_find_by_json_key(nvi->string);
        if (!d) continue;

        switch (d->type) {
        case PRJCFG_NETVAR_TYPE_BOOL: {
            bool value = cJSON_IsTrue(nvi);
            if (value != *(bool *)d->ptr) {
                *(bool *)d->ptr = value;
                if (out_nvs_changed) *out_nvs_changed = true;
            }
            break;
        }
        case PRJCFG_NETVAR_TYPE_U8: {
            uint8_t value = (uint8_t)nvi->valueint;
            if (value != *(uint8_t *)d->ptr) {
                *(uint8_t *)d->ptr = value;
                if (out_nvs_changed) *out_nvs_changed = true;
            }
            break;
        }
        case PRJCFG_NETVAR_TYPE_I8: {
            int8_t value = (int8_t)nvi->valueint;
            if (value != *(int8_t *)d->ptr) {
                *(int8_t *)d->ptr = value;
                if (out_nvs_changed) *out_nvs_changed = true;
            }
            break;
        }
        case PRJCFG_NETVAR_TYPE_I32: {
            int32_t value = (int32_t)nvi->valueint;
            if (value != *(int32_t *)d->ptr) {
                *(int32_t *)d->ptr = value;
                if (out_nvs_changed) *out_nvs_changed = true;
            }
            break;
        }
        case PRJCFG_NETVAR_TYPE_U32: {
            uint32_t value = (uint32_t)nvi->valueint;
            if (value != *(uint32_t *)d->ptr) {
                *(uint32_t *)d->ptr = value;
                if (out_nvs_changed) *out_nvs_changed = true;
            }
            break;
        }
        case PRJCFG_NETVAR_TYPE_STRING:
            // TODO: strncpy con límite si quieres que JSON actualice strings
            break;
        }
    }
}
