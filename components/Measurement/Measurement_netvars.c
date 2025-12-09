#include <string.h>
#include <Measurement.h>
#include "Measurement_netvars.h"


#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

const static char TAG[] = "Measurement_netvars";

// La instancia de datos real debe estar definida en otro lugar, por ejemplo:
// MEASUREMENT_t MEASUREMENT;
extern Measurement_dre_t Measurement_dre;

static SemaphoreHandle_t s_nvs_mutex = NULL;
static bool s_nvs_dirty = false;
static TickType_t s_nvs_dirty_since = 0;

static inline BaseType_t _create_nvs_mutex_once(void)
{
    if (!s_nvs_mutex)
    {
        s_nvs_mutex = xSemaphoreCreateMutex();
        if (!s_nvs_mutex) return pdFAIL;
    }
    return pdPASS;
}

const NetVars_desc_t Measurement_netvars_desc[] = {
#include "Measurement_netvars_fragment.c_"
};

const size_t Measurement_netvars_count = sizeof(Measurement_netvars_desc) / sizeof(Measurement_netvars_desc[0]);

void Measurement_netvars_append_json(cJSON *root)
{
    if (Measurement_netvars_count > 0)
    {
        NetVars_append_json_component("Measurement", Measurement_netvars_desc, Measurement_netvars_count, root);
    }
}

void Measurement_netvars_nvs_load(void)
{
    NetVars_nvs_load_component("Measurement", Measurement_netvars_desc, Measurement_netvars_count);
}

void Measurement_netvars_nvs_save(void)
{
    NetVars_nvs_save_component("Measurement", Measurement_netvars_desc, Measurement_netvars_count);
}

void Measurement_config_parse_json(const char *data)
{
    bool nvs_cfg_changed = NetVars_parse_json_component_data("Measurement", Measurement_netvars_desc, Measurement_netvars_count, data);
        if (nvs_cfg_changed)
        {
            Measurement_nvs_set_dirty();
    }
}


void Measurement_nvs_set_dirty(void)
{
    if (_create_nvs_mutex_once() != pdPASS) return;
    xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);
    s_nvs_dirty = true;
    s_nvs_dirty_since = xTaskGetTickCount();
    xSemaphoreGive(s_nvs_mutex);
}

void Measurement_nvs_spin(void)
{
    if (_create_nvs_mutex_once() != pdPASS) return;
    TickType_t now_ticks = xTaskGetTickCount();
    bool should_save = false;
    xSemaphoreTake(s_nvs_mutex, portMAX_DELAY);
    if (s_nvs_dirty && (TickType_t)(now_ticks - s_nvs_dirty_since) >= pdMS_TO_TICKS(5000))
    {
        s_nvs_dirty = false;
        should_save = true;
    }
    xSemaphoreGive(s_nvs_mutex);
    if (should_save)
    {
        Measurement_netvars_nvs_save();
    }
}
