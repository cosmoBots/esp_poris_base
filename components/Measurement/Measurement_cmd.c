#include <string.h>

#include <cJSON.h>

#include <nvs_flash.h>
#include <esp_system.h>
#include <esp_log.h>

#include "PrjCfg.h"

#include "Measurement.h"
#include "Measurement_nvs.h"

static char TAG[] = "Measurement_cmd";

static bool nvs_cfg_changed = false;

#if CONFIG_MEASUREMENT_USE_THREAD
static Measurement_dre_t comm_dre;
static Measurement_dre_t *cdre = &comm_dre;
#else
static Measurement_dre_t *cdre = &Measurement_dre;
#endif

static void config_parse_json_dict(cJSON *root)
{
    cJSON *nvi = NULL;

    cJSON_ArrayForEach(nvi, root)
    {
        int len = strlen(nvi->string);

        ESP_LOGI(TAG, "string %s len %d", nvi->string, len);

        bool found_key = false;

        if (!found_key && strcmp(nvi->string, "measdumm") == 0)
        {
            ESP_LOGI(TAG, "Recognized %s", nvi->string);
            bool value = cJSON_IsTrue(nvi);
            ESP_LOGI(TAG, "Parsed value: %hu", value);
            found_key = true;
        }
    }
}

static void config_parse_json(const char *data)
{
    nvs_cfg_changed = false;

    cJSON *root = cJSON_Parse(data);

    if (root != NULL)
    {
        if (data[0] == '[')
        {
            cJSON *nvi = NULL;
            cJSON_ArrayForEach(nvi, root)
            {
                config_parse_json_dict(nvi);
            }
        }
        else
        {
            config_parse_json_dict(root);
        }
        cJSON_Delete(root);
        if (nvs_cfg_changed)
        {
            Measurement_nvs_cfg_save();
        }
    }
}

void Measurement_compose_json_payload(cJSON *root)
{
#if CONFIG_MEASUREMENT_USE_THREAD
    if (Measurement_get_dre_clone(cdre) != Measurement_ret_ok)
    {
        ESP_LOGE(TAG, "Could not obtain a DRE clone to compose the JSON payload");
    }
    else
#endif
    {
        cJSON_AddNumberToObject(root, "ai-1", cdre->ai1);
        cJSON_AddNumberToObject(root, "ai-2", cdre->ai2);
        cJSON_AddBoolToObject(root, "bi-0", cdre->bi0);
    }
}

void Measurement_parse_callback(const char *data, int len)
{
    ESP_LOGI(TAG, "Parsing the CFG payload %d %.*s", len, len, data);
    config_parse_json(data);
}

void Measurement_req_parse_callback(const char *data, int len)
{
    ESP_LOGI(TAG, "Parsing the REQ payload %d %.*s", len, len, data);
}

static uint32_t msg_counter = 0;
void Measurement_compose_callback(char *data, int *len)
{
    sprintf((char *)data, "this is a payload (%lu)", msg_counter++);
    *len = strlen((char *)data);
    ESP_LOGI(TAG, "Composing the DATA payload %d %.*s", *len, *len, data);

    cJSON *root = cJSON_CreateObject();
    Measurement_compose_json_payload(root);

    char *cPayload = cJSON_PrintUnformatted(root);
    if (cPayload != NULL)
    {
        size_t length = strlen(cPayload);
        *len = (int)length;
        ESP_LOGI(TAG, "LEN %d", length);
        ESP_LOGI(TAG, "Payload %s", cPayload);
        strncpy(data, cPayload, length);
        free(cPayload);
    }
    else
    {
        data[0] = 'a';
        data[1] = '\0';
        *len = 1;
    }
    cJSON_Delete(root);
}
