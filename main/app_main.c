/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include <esp_log.h>

#include <PrjCfg.h>
#ifdef CONFIG_PORIS_ENABLE_OTCOAP
#include <OtCoap.h>
#endif
#ifdef CONFIG_PORIS_ENABLE_HELLOWORLD
#include <HelloWorld.h>
#endif

typedef enum
{
    app_main_ret_error = -1,
    app_main_ret_ok = 0
} app_main_return_code;

static char TAG[] = "main";

app_main_return_code init_components(void)
{
    app_main_return_code ret = app_main_ret_error;
    if (PrjCfg_setup() == PrjCfg_ret_ok)
    {
#ifdef CONFIG_PORIS_ENABLE_OTCOAP
        if (OtCoap_setup() == OtCoap_ret_ok)
        {
            ret = app_main_ret_ok;
        }
#else
        ret = app_main_ret_ok;
#endif
#ifdef CONFIG_PORIS_ENABLE_HELLOWORLD
        if (ret == app_main_ret_ok)
        {
            if (HelloWorld_setup() != HelloWorld_ret_ok)
            {
                ret = app_main_ret_error;
            }
        }
#endif
    }
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
#endif

    error_accumulator |= error_occurred;

#ifdef CONFIG_PORIS_ENABLE_HELLOWORLD
    error_occurred = (HelloWorld_enable() != HelloWorld_ret_ok);
#ifdef CONFIG_HELLOWORLD_USE_THREAD
    error_occurred |= (HelloWorld_start() != HelloWorld_ret_ok);
#endif
#endif

    error_accumulator |= error_occurred;

#ifdef CONFIG_PORIS_ENABLE_OTCOAP
    error_occurred = (OtCoap_enable() != OtCoap_ret_ok);
#ifdef CONFIG_OTCOAP_USE_THREAD
    error_occurred |= (OtCoap_start() != OtCoap_ret_ok);
#endif
#endif
    if (error_accumulator)
    {
        ret = app_main_ret_error;
    }
    return ret;
}

#ifdef CONFIG_PORIS_ENABLE_HELLOWORLD
#ifdef CONFIG_HELLOWORLD_USE_THREAD
static void hw_safe()
{
    ESP_LOGI(TAG, "Safe calling hw is enabled? %d", HelloWorld_dre.enabled);
}
#endif
#endif

app_main_return_code run_components(void)
{
    app_main_return_code ret = app_main_ret_ok;
    bool error_accumulator = false;

#ifdef CONFIG_PORIS_ENABLE_PRJCFG
#ifndef CONFIG_PRJCFG_USE_THREAD
    error_accumulator |= (PrjCfg_spin() != PrjCfg_ret_ok);
#endif
#endif

#ifdef CONFIG_PORIS_ENABLE_HELLOWORLD
#ifndef CONFIG_HELLOWORLD_USE_THREAD
    error_accumulator |= (HelloWorld_spin() != HelloWorld_ret_ok);
#else
    HelloWorld_execute_function_safemode(hw_safe);
#endif
#endif

#ifdef CONFIG_PORIS_ENABLE_OTCOAP
#ifndef CONFIG_OTCOAP_USE_THREAD
    error_accumulator |= (OtCoap_spin() != OtCoap_ret_ok);
#endif
#endif
    if (error_accumulator)
    {
        ret = app_main_ret_error;
    }
    return ret;
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
    while (true)
    {
        ESP_LOGI(TAG, "app_main spinning");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (shall_execute)
        {
            if (run_components() != app_main_ret_ok)
            {
                ESP_LOGW(TAG, "Could not run all  components!!!");
            }
        }
    }
}
