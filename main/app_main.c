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

#include <nvs_flash.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include <esp_log.h>

// Include project configuration
#include <PrjCfg.h>


// Include components
#ifdef CONFIG_PORIS_ENABLE_WIFI
#include <Wifi.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
#include <OTA.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_MQTTCOMM
#include <MQTTComm.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_MEASUREMENT
#include <Measurement.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLED
#include <DualLED.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLEDTESTER
#include <DualLedTester.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYS
#include <Relays.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYSTEST
#include <RelaysTest.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_TOUCHSCREEN
#include <TouchScreen.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_PROVISIONING
#include <Provisioning.h>
#endif
// [PORIS_INTEGRATION_INCLUDE]

// Include comms callbacks
#ifdef CONFIG_PORIS_ENABLE_PRJCFG
#include <PrjCfg_netvars.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_MEASUREMENT
#include <Measurement_netvars.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_MQTTCOMM
#include <MQTTComm_netvars.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
#include <OTA_netvars.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLED
#include <DualLED_netvars.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLEDTESTER
#include <DualLedTester_netvars.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYS
#include <Relays_netvars.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYSTEST
#include <RelaysTest_netvars.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_UDPCOMM
#include <UDPComm_netvars.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_WIFI
#include <Wifi_netvars.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_TOUCHSCREEN
#include <TouchScreen_netvars.h>
#endif
// [PORIS_INTEGRATION_NETVARS_INCLUDE]

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
#ifdef CONFIG_PORIS_ENABLE_PROVISIONING
    error_occurred = (Provisioning_setup() != Provisioning_ret_ok);
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
#ifdef CONFIG_PORIS_ENABLE_DUALLED
    error_occurred = (DualLED_setup() != DualLED_ret_ok);
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLEDTESTER
    error_occurred = (DualLedTester_setup() != DualLedTester_ret_ok);
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYS
    error_occurred = (Relays_setup() != Relays_ret_ok);
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYSTEST
    error_occurred = (RelaysTest_setup() != RelaysTest_ret_ok);
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_TOUCHSCREEN
    error_occurred = (TouchScreen_setup() != TouchScreen_ret_ok);
    error_accumulator |= error_occurred;
#endif
// [PORIS_INTEGRATION_INIT]
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
#ifdef CONFIG_PORIS_ENABLE_PROVISIONING
    error_occurred = (Provisioning_enable() != Provisioning_ret_ok);
#ifdef CONFIG_PROVISIONING_USE_THREAD
    if (!error_occurred)
    {
        error_occurred |= (Provisioning_start() != Provisioning_ret_ok);
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
#ifdef CONFIG_PORIS_ENABLE_DUALLED
    error_occurred = (DualLED_enable() != DualLED_ret_ok);
#ifdef CONFIG_DUALLED_USE_THREAD
    if (!error_occurred)
    {
        error_occurred |= (DualLED_start() != DualLED_ret_ok);
    }
#endif
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLEDTESTER
    error_occurred = (DualLedTester_enable() != DualLedTester_ret_ok);
#ifdef CONFIG_DUALLEDTESTER_USE_THREAD
    if (!error_occurred)
    {
        error_occurred |= (DualLedTester_start() != DualLedTester_ret_ok);
    }
#endif
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYS
    error_occurred = (Relays_enable() != Relays_ret_ok);
#ifdef CONFIG_RELAYS_USE_THREAD
    if (!error_occurred)
    {
        error_occurred |= (Relays_start() != Relays_ret_ok);
    }
#endif
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYSTEST
    error_occurred = (RelaysTest_enable() != RelaysTest_ret_ok);
#ifdef CONFIG_RELAYSTEST_USE_THREAD
    if (!error_occurred)
    {
        error_occurred |= (RelaysTest_start() != RelaysTest_ret_ok);
    }
#endif
    error_accumulator |= error_occurred;
#endif
#ifdef CONFIG_PORIS_ENABLE_TOUCHSCREEN
    error_occurred = (TouchScreen_enable() != TouchScreen_ret_ok);
#ifdef CONFIG_TOUCHSCREEN_USE_THREAD
    if (!error_occurred)
    {
        error_occurred |= (TouchScreen_start() != TouchScreen_ret_ok);
    }
#endif
    error_accumulator |= error_occurred;
#endif
// [PORIS_INTEGRATION_START]
    if (error_accumulator)
    {
        ret = app_main_ret_error;
    }
    return ret;
}

#define MEASUREMENT_CYCLE_LIMIT ((MEASUREMENT_CYCLE_PERIOD_MS / MAIN_CYCLE_PERIOD_MS) - 1)
#define MQTTCOMM_CYCLE_LIMIT ((MQTTCOMM_CYCLE_PERIOD_MS / MAIN_CYCLE_PERIOD_MS) - 1)
#define DUALLED_CYCLE_PERIOD_MS 100
#define DUALLED_CYCLE_LIMIT ((DUALLED_CYCLE_PERIOD_MS / MAIN_CYCLE_PERIOD_MS) - 1)
#define DUALLEDTESTER_CYCLE_PERIOD_MS 100
#define DUALLEDTESTER_CYCLE_LIMIT ((DUALLEDTESTER_CYCLE_PERIOD_MS / MAIN_CYCLE_PERIOD_MS) - 1)
#define RELAYS_CYCLE_PERIOD_MS 100
#define RELAYS_CYCLE_LIMIT ((RELAYS_CYCLE_PERIOD_MS / MAIN_CYCLE_PERIOD_MS) - 1)
#define RELAYSTEST_CYCLE_PERIOD_MS 100
#define RELAYSTEST_CYCLE_LIMIT ((RELAYSTEST_CYCLE_PERIOD_MS / MAIN_CYCLE_PERIOD_MS) - 1)
#define TOUCHSCREEN_CYCLE_PERIOD_MS 1000
#define TOUCHSCREEN_CYCLE_LIMIT_MS ((TOUCHSCREEN_CYCLE_PERIOD_MS / MAIN_CYCLE_PERIOD_MS) - 1)
// [PORIS_INTEGRATION_DEFINES]

static uint16_t mqttcomm_cycle_counter = 0;
static uint16_t measurement_cycle_counter = 0;
static uint16_t dualled_cycle_counter = 0;
static uint16_t dualledtester_cycle_counter = 0;
static uint16_t relays_cycle_counter = 0;
static uint16_t relaystest_cycle_counter = 0;
static uint8_t touchscreen_cycle_counter = 0;
// [PORIS_INTEGRATION_COUNTERS]

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
#ifdef CONFIG_PORIS_ENABLE_DUALLED
#ifndef CONFIG_DUALLED_USE_THREAD
    if (dualled_cycle_counter <= 0)
    {
        error_accumulator |= (DualLED_spin() != DualLED_ret_ok);
        dualled_cycle_counter = DUALLED_CYCLE_LIMIT;
    }
    else
    {
        dualled_cycle_counter--;
    }
#endif
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYS
#ifndef CONFIG_RELAYS_USE_THREAD
    if (relays_cycle_counter <= 0)
    {
        error_accumulator |= (Relays_spin() != Relays_ret_ok);
        relays_cycle_counter = RELAYS_CYCLE_LIMIT;
    }
    else
    {
        relays_cycle_counter--;
    }
#endif
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYSTEST
#ifndef CONFIG_RELAYSTEST_USE_THREAD
    if (relaystest_cycle_counter <= 0)
    {
        error_accumulator |= (RelaysTest_spin() != RelaysTest_ret_ok);
        relaystest_cycle_counter = RELAYSTEST_CYCLE_LIMIT;
    }
    else
    {
        relaystest_cycle_counter--;
    }
#endif
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLEDTESTER
#ifndef CONFIG_DUALLEDTESTER_USE_THREAD
    if (dualledtester_cycle_counter <= 0)
    {
        error_accumulator |= (DualLedTester_spin() != DualLedTester_ret_ok);
        dualledtester_cycle_counter = DUALLEDTESTER_CYCLE_LIMIT;
    }
    else
    {
        dualledtester_cycle_counter--;
    }
#endif
#endif
#ifdef CONFIG_PORIS_ENABLE_TOUCHSCREEN
#ifndef CONFIG_TOUCHSCREEN_USE_THREAD
    if (touchscreen_cycle_counter <= 0)
    {
        error_accumulator |= (TouchScreen_spin() != TouchScreen_ret_ok);
        touchscreen_cycle_counter = TOUCHSCREEN_CYCLE_LIMIT_MS;
    }
    else
    {
        touchscreen_cycle_counter--;
    }
#endif
#endif
// [PORIS_INTEGRATION_RUN]
    if (error_accumulator)
    {
        ret = app_main_ret_error;
    }
    return ret;
}


void main_parse_callback(const char *data, int len)
{
    ESP_LOGI(TAG, "Parsing the CFG payload %d %.*s", len, len, data);
#ifdef CONFIG_PORIS_ENABLE_PRJCFG
    PrjCfg_config_parse_json(data);
#endif
#ifdef CONFIG_PORIS_ENABLE_MEASUREMENT
    Measurement_config_parse_json(data);
#endif
#ifdef CONFIG_PORIS_ENABLE_MQTTCOMM
    MQTTComm_config_parse_json(data);
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
    OTA_config_parse_json(data);
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLED
    DualLED_config_parse_json(data);
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLEDTESTER
    DualLedTester_config_parse_json(data);
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYS
    Relays_config_parse_json(data);
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYSTEST
    RelaysTest_config_parse_json(data);
#endif
#ifdef CONFIG_PORIS_ENABLE_UDPCOMM
    UDPComm_config_parse_json(data);
#endif
#ifdef CONFIG_PORIS_ENABLE_WIFI
    Wifi_config_parse_json(data);
#endif
#ifdef CONFIG_PORIS_ENABLE_TOUCHSCREEN
    TouchScreen_config_parse_json(data);
#endif
// [PORIS_INTEGRATION_NETVARS_PARSE]
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
#ifdef CONFIG_PORIS_ENABLE_PRJCFG
    PrjCfg_netvars_append_json(root);
#endif
#ifdef CONFIG_PORIS_ENABLE_MEASUREMENT
    Measurement_netvars_append_json(root);
#endif
#ifdef CONFIG_PORIS_ENABLE_MQTTCOMM
    MQTTComm_netvars_append_json(root);
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
    OTA_netvars_append_json(root);
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLED
    DualLED_netvars_append_json(root);
#endif
#ifdef CONFIG_PORIS_ENABLE_DUALLEDTESTER
    DualLedTester_netvars_append_json(root);
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYS
    Relays_netvars_append_json(root);
#endif
#ifdef CONFIG_PORIS_ENABLE_RELAYSTEST
    RelaysTest_netvars_append_json(root);
#endif
#ifdef CONFIG_PORIS_ENABLE_UDPCOMM
    UDPComm_netvars_append_json(root);
#endif
#ifdef CONFIG_PORIS_ENABLE_WIFI
    Wifi_netvars_append_json(root);
#endif
#ifdef CONFIG_PORIS_ENABLE_TOUCHSCREEN
    TouchScreen_netvars_append_json(root);
#endif
// [PORIS_INTEGRATION_NETVARS_APPEND]

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

    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

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
        mqtt_comm_cfg_t cfg = {0};
        cfg.f_cfg_cb = main_parse_callback;
        cfg.f_req_cb = main_req_parse_callback;
        cfg.f_data_cb = main_compose_callback;

        MQTTComm_setup(&cfg);
        ESP_LOGI(TAG, "---> MQTT topics for device %s", MQTTComm_dre.device);
        ESP_LOGI(TAG, "     cfg : %s", MQTTComm_dre.cfg_topic);
        ESP_LOGI(TAG, "     cmd : %s", MQTTComm_dre.req_topic);
        ESP_LOGI(TAG, "     data: %s", MQTTComm_dre.data_topic);
#ifndef CONFIG_MQTTCOMM_USE_THREAD
        MQTTComm_enable();
#endif
    }
    while (true)
    {
        //ESP_LOGI(TAG, "app_main spinning");
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
