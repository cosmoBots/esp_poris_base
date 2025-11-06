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

typedef enum {
    app_main_ret_error = -1,
    app_main_ret_ok = 0
}app_main_return_code;

static char TAG[] = "main";

app_main_return_code init_components(void)
{
    app_main_return_code ret = app_main_ret_error;
    if (PrjCfg_setup() == PrjCfg_ret_ok)
    {
        #ifdef CONFIG_PORIS_ENABLE_OTCOAP
        if (PrjCfg_setup() == PrjCfg_ret_ok)
        {
            ret = app_main_ret_ok;
        }
        #else
            ret = app_main_ret_ok;
        #endif        
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
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    if (init_components() != app_main_ret_ok)
    {
        ESP_LOGE(TAG,"Cannot init components!!!");
    }
    while (true)
    {
        ESP_LOGI(TAG,"app_main spinning");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}
