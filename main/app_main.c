/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <cJSON.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include <esp_log.h>

// Include project configuration
#include <PrjCfg.h>


// Include components
#include <Wifi.h>
#include <OTA.h>
#include <MQTTComm.h>
#include <Measurement.h>

// Include comms callbacks
#include <PrjCfg_cmd.h>
#include <Measurement_cmd.h>

typedef enum
{
    app_main_ret_error = -1,
    app_main_ret_ok = 0
} app_main_return_code;

static char TAG[] = "main";

app_main_return_code init_components(void)
{
    app_main_return_code ret = app_main_ret_ok;
    bool error_occurred = false;
    bool error_accumulator = false;

#ifdef CONFIG_PORIS_ENABLE_PRJCFG
    error_occurred = (PrjCfg_setup() != PrjCfg_ret_ok);
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_WIFI
    error_occurred = (Wifi_setup() != Wifi_ret_ok);
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
    error_occurred = (OTA_setup() != OTA_ret_ok);
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_MEASUREMENT
    error_occurred = (Measurement_setup() != Measurement_ret_ok);
    error_accumulator |= error_occurred;
#endif
    return ret;
}

app_main_return_code start_components(void)
{
    app_main_return_code ret = app_main_ret_ok;
    bool error_occurred = false;
    bool error_accumulator = false;

#ifdef CONFIG_PORIS_ENABLE_PRJCFG
    error_occurred = (PrjCfg_enable() != PrjCfg_ret_ok);
#ifdef CONFIG_PRJCFG_USE_THREAD
    if (!error_occurred)
    {
        error_occurred |= (PrjCfg_start() != PrjCfg_ret_ok);
    }
#endif
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_WIFI
    // Wifi does not need starting, setup already did it
    // only on-demand
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
    // OTA Should not be started with the rest of the components
    // only on-demand
#endif
#ifdef CONFIG_PORIS_ENABLE_MQTTCOMM
    error_occurred = (MQTTComm_enable() != MQTTComm_ret_ok);
#ifdef CONFIG_MQTTCOMM_USE_THREAD
    if (!error_occurred)
    {
        error_occurred |= (MQTTComm_start() != MQTTComm_ret_ok);
    }
#endif
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_MEASUREMENT
    error_occurred = (Measurement_enable() != Measurement_ret_ok);
#ifdef CONFIG_MEASUREMENT_USE_THREAD
    if (!error_occurred)
    {
        error_occurred |= (Measurement_start() != Measurement_ret_ok);
    }
#endif
    error_accumulator |= error_occurred;
#endif
    if (error_accumulator)
    {
        ret = app_main_ret_error;
    }
    return ret;
}

#define MEASUREMENT_CYCLE_LIMIT ((MEASUREMENT_CYCLE_PERIOD_MS / MAIN_CYCLE_PERIOD_MS) - 1)
#define MQTTCOMM_CYCLE_LIMIT ((MQTTCOMM_CYCLE_PERIOD_MS / MAIN_CYCLE_PERIOD_MS) - 1)

static uint8_t mqttcomm_cycle_counter = 0;
static uint8_t measurement_cycle_counter = 0;

app_main_return_code run_components(void)
{
    app_main_return_code ret = app_main_ret_ok;
    bool error_accumulator = false;

#ifdef CONFIG_PORIS_ENABLE_PRJCFG
#ifndef CONFIG_PRJCFG_USE_THREAD
    error_accumulator |= (PrjCfg_spin() != PrjCfg_ret_ok);
#endif
#endif
#ifdef CONFIG_PORIS_ENABLE_WIFI
    error_accumulator |= (Wifi_spin() != Wifi_ret_ok);
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
    // OTA does not use spin
#endif
#ifdef CONFIG_PORIS_ENABLE_MQTTCOMM
#ifndef CONFIG_MQTTCOMM_USE_THREAD
    if (mqttcomm_cycle_counter <= 0)
    {
        error_accumulator |= (MQTTComm_spin() != MQTTComm_ret_ok);
        mqttcomm_cycle_counter = MQTTCOMM_CYCLE_LIMIT;
    }
    else
    {
        mqttcomm_cycle_counter--;
    }
#endif
#endif
#ifdef CONFIG_PORIS_ENABLE_MEASUREMENT
#ifndef CONFIG_MEASUREMENT_USE_THREAD
    if (measurement_cycle_counter <= 0)
    {
        error_accumulator |= (Measurement_spin() != Measurement_ret_ok);
        measurement_cycle_counter = MEASUREMENT_CYCLE_LIMIT;
    }
    else
    {
        measurement_cycle_counter--;
    }
#endif
#endif
    if (error_accumulator)
    {
        ret = app_main_ret_error;
    }
    return ret;
}


void main_parse_callback(const char *data, int len)
{
    ESP_LOGI(TAG, "Parsing the CFG payload %d %.*s", len, len, data);
    PrjCfg_parse_callback(data, len);
    Measurement_parse_callback(data, len);
}

void main_req_parse_callback(const char *data, int len)
{
    ESP_LOGI(TAG, "Parsing the REQ payload %d %.*s", len, len, data);
    if (len == 1)
    {
        if (data[0] == 'r')
        {
            esp_restart();
        }
        if (data[0] == 'u')
        {
            OTA_enable();
            OTA_start();
        }
    }
}

static uint32_t msg_counter = 0;
void main_compose_callback(char *data, int *len)
{
    sprintf((char *)data, "this is a payload (%lu)", msg_counter++);
    *len = strlen((char *)data);
    ESP_LOGI(TAG, "Composing the DATA payload %d %.*s", *len, *len, data);

    cJSON *root = cJSON_CreateObject();
    PrjCfg_compose_json_payload(root);
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

#define MQTT_TOPIC_SITE "Pete'sHouse"
#define MQTT_TOPIC_DEVICE "HVAC-4"

void app_main(void)
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    bool shall_execute = true;
    if (init_components() != app_main_ret_ok)
    {
        ESP_LOGE(TAG, "Cannot init components!!!");
        shall_execute = false;
    }
    if (shall_execute)
    {
        if (start_components() != app_main_ret_ok)
        {
            ESP_LOGE(TAG, "Cannot start components!!!");
            shall_execute = false;
        }
    }
    if (shall_execute)
    {
        // Boot-time actions
        // Check OTA
        OTA_enable();
        OTA_start();
        // If OTA has not rebooted, we should continue
        OTA_disable();

        // Now let's setup the MQTT topics
        mqtt_comm_cfg_t cfg;
        
        cfg.f_cfg_cb = main_parse_callback;
        cfg.f_req_cb = main_req_parse_callback;
        cfg.f_data_cb = main_compose_callback;

        cfg.cfg_client_id = MQTT_TOPIC_DEVICE;
        sprintf(cfg.cfg_topic, "IceEnergy/v1/%s/%s/configuration", MQTT_TOPIC_SITE, MQTT_TOPIC_DEVICE);
        sprintf(cfg.req_topic, "IceEnergy/v1/%s/%s/commands", MQTT_TOPIC_SITE, MQTT_TOPIC_DEVICE);
        sprintf(cfg.data_topic, "IceEnergy/v1/%s/%s/measurement", MQTT_TOPIC_SITE, MQTT_TOPIC_DEVICE);

        ESP_LOGI(TAG,"---> TOPICS RELATED TO %s", cfg.cfg_client_id);

        MQTTComm_setup(&cfg);
#ifndef CONFIG_MQTTCOMM_USE_THREAD
        MQTTComm_enable();
#endif
    }
    while (true)
    {
        ESP_LOGI(TAG, "app_main spinning");
        vTaskDelay(MAIN_CYCLE_PERIOD_MS / portTICK_PERIOD_MS);
        if (shall_execute)
        {
            if (run_components() != app_main_ret_ok)
            {
                ESP_LOGW(TAG, "Could not run all  components!!!");
            }
        }
    }
}
