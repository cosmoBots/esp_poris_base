// BEGIN --- Standard C headers section ---
#include <stdio.h>
#include <string.h>

// END   --- Standard C headers section ---

// BEGIN --- SDK config section---
#include <sdkconfig.h>
// END   --- SDK config section---

// BEGIN --- FreeRTOS headers section ---
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#if CONFIG_PRJCFG_USE_THREAD
  #include <freertos/semphr.h>
#endif

// END   --- FreeRTOS headers section ---


// BEGIN --- ESP-IDF headers section ---
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_mac.h>
#include <cJSON.h>

// END   --- ESP-IDF headers section ---

// BEGIN --- Project configuration section ---
#include <PrjCfg.h> // Including project configuration module 
// END   --- Project configuration section ---

// BEGIN --- Project configuration section ---

// end   --- Project configuration section ---

// BEGIN --- Self-includes section ---
#include "PrjCfg.h"

// END --- Self-includes section ---

// BEGIN --- Logging related variables
static const char *TAG = "PrjCfg_nvs";
// END --- Logging related variables

void PrjCfg_nvs_cfg_load(void)
{
    esp_err_t err;
    // Open

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("cfg", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        int8_t aux;
#ifdef CONFIG_PORIS_ENABLE_BLECENTRAL
        ESP_LOGI(TAG, "Reading central_role from NVS ... ");
        err = nvs_get_u8(my_handle, "rcentr", (uint8_t *)&PrjCfg_dre.central_role);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.central_role);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The central_role value is not initialized yet, using %d!", PrjCfg_dre.central_role);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading central_role, using %d!", esp_err_to_name(err), PrjCfg_dre.central_role);
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_BLEPERIPHERAL
        ESP_LOGI(TAG, "Reading peripheral_role from NVS ... ");
        err = nvs_get_u8(my_handle, "rperiph", (uint8_t *)&PrjCfg_dre.peripheral_role);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.peripheral_role);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The peripheral_role value is not initialized yet, using %d!", PrjCfg_dre.peripheral_role);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading peripheral_role, using %d!", esp_err_to_name(err), PrjCfg_dre.peripheral_role);
        }

        ESP_LOGI(TAG, "Reading echo parameter from NVS ... ");
        err = nvs_get_u8(my_handle, "echo", (uint8_t *)&PrjCfg_dre.echo);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.echo);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The echo value is not initialized yet, using %d!", PrjCfg_dre.echo);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading echo, using %d!", esp_err_to_name(err), PrjCfg_dre.echo);
        }
#endif

#ifdef CONFIG_PORIS_ENABLE_OTA
        ESP_LOGI(TAG, "Reading skip_ota from NVS ... ");
        err = nvs_get_u8(my_handle, "otaskip", (uint8_t *)&PrjCfg_dre.skip_ota);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.skip_ota);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The skip_ota value is not initialized yet, using %d!", PrjCfg_dre.skip_ota);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading skip_ota, using %d!", esp_err_to_name(err), PrjCfg_dre.skip_ota);
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTBRIDGE
        ESP_LOGI(TAG, "Reading uart_bridge from NVS ... ");
        err = nvs_get_u8(my_handle, "uartbr", (uint8_t *)&PrjCfg_dre.uart_bridge);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_bridge);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_bridge value is not initialized yet, using %d!", PrjCfg_dre.uart_bridge);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_bridge, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_bridge);
        }
#endif

#ifdef CONFIG_PORIS_ENABLE_UARTUSER
        ESP_LOGI(TAG, "Reading uart_user_port parameter from NVS ... ");
        err = nvs_get_u8(my_handle, "uuport", (uint8_t *)&PrjCfg_dre.uart_user_port);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_user_port);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_user_port value is not initialized yet, using %d!", PrjCfg_dre.uart_user_port);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_user_port, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_user_port);
        }


        ESP_LOGI(TAG, "Reading uart_user_baudrate parameter from NVS ... ");
        err = nvs_get_i32(my_handle, "uubaud", (int32_t *)&PrjCfg_dre.uart_user_baudrate);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_user_baudrate);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_user_baudrate value is not initialized yet, using %d!", PrjCfg_dre.uart_user_baudrate);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_user_baudrate, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_user_baudrate);
        }

        ESP_LOGI(TAG, "Reading uart_user_stop_bits parameter from NVS ... ");
        err = nvs_get_u8(my_handle, "uustopb", (uint8_t *)&PrjCfg_dre.uart_user_stop_bits);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_user_stop_bits);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_user_stop_bits value is not initialized yet, using %d!", PrjCfg_dre.uart_user_stop_bits);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_user_stop_bits, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_user_stop_bits);
        }

        ESP_LOGI(TAG, "Reading uart_user_rx_io_num parameter from NVS ... ");
        err = nvs_get_i8(my_handle, "uurx", &aux);
        switch (err)
        {
        case ESP_OK:
            PrjCfg_dre.uart_user_rx_io_num = (gpio_num_t)aux;
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_user_rx_io_num);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_user_rx_io_num value is not initialized yet, using %d!", PrjCfg_dre.uart_user_rx_io_num);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_user_rx_io_num, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_user_rx_io_num);
        }

        ESP_LOGI(TAG, "Reading uart_user_tx_io_num parameter from NVS ... ");
        err = nvs_get_i8(my_handle, "uutx", &aux);
        switch (err)
        {
        case ESP_OK:
            PrjCfg_dre.uart_user_tx_io_num = (gpio_num_t)aux;
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_user_tx_io_num);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_user_tx_io_num value is not initialized yet, using %d!", PrjCfg_dre.uart_user_tx_io_num);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_user_tx_io_num, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_user_tx_io_num);
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTPERIPH
        ESP_LOGI(TAG, "Reading uart_periph_port parameter from NVS ... ");
        err = nvs_get_u8(my_handle, "upport", (uint8_t *)&PrjCfg_dre.uart_periph_port);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_periph_port);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_periph_port value is not initialized yet, using %d!", PrjCfg_dre.uart_periph_port);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_periph_port, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_periph_port);
        }
        ESP_LOGI(TAG, "Reading uart_periph_baudrate parameter from NVS ... ");
        err = nvs_get_i32(my_handle, "upbaud", (int32_t *)&PrjCfg_dre.uart_periph_baudrate);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_periph_baudrate);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_periph_baudrate value is not initialized yet, using %d!", PrjCfg_dre.uart_periph_baudrate);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_periph_baudrate, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_periph_baudrate);
        }
        ESP_LOGI(TAG, "Reading uart_periph_stop_bits parameter from NVS ... ");
        err = nvs_get_u8(my_handle, "upstopb", (uint8_t *)&PrjCfg_dre.uart_periph_stop_bits);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_periph_stop_bits);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_periph_stop_bits value is not initialized yet, using %d!", PrjCfg_dre.uart_periph_stop_bits);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_periph_stop_bits, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_periph_stop_bits);
        }

        ESP_LOGI(TAG, "Reading uart_periph_rx_io_num parameter from NVS ... ");
        err = nvs_get_i8(my_handle, "uprx", &aux);
        switch (err)
        {
        case ESP_OK:
            PrjCfg_dre.uart_periph_rx_io_num = (gpio_num_t)aux;
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_periph_rx_io_num);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_periph_rx_io_num value is not initialized yet, using %d!", PrjCfg_dre.uart_periph_rx_io_num);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_periph_rx_io_num, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_periph_rx_io_num);
        }

        ESP_LOGI(TAG, "Reading uart_periph_tx_io_num parameter from NVS ... ");
        err = nvs_get_i8(my_handle, "uptx", &aux);
        switch (err)
        {
        case ESP_OK:
            PrjCfg_dre.uart_periph_tx_io_num = (gpio_num_t)aux;
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_periph_tx_io_num);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_periph_tx_io_num value is not initialized yet, using %d!", PrjCfg_dre.uart_periph_tx_io_num);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_periph_tx_io_num, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_periph_tx_io_num);
        }
#endif

#ifdef CONFIG_PORIS_ENABLE_BLINK
        ESP_LOGI(TAG, "Reading blink from NVS ... ");
        err = nvs_get_u8(my_handle, "blink", (uint8_t *)&PrjCfg_dre.blink_enabled);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.blink_enabled);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The blink value is not initialized yet, using %d!", PrjCfg_dre.blink_enabled);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading blink, using %d!", esp_err_to_name(err), PrjCfg_dre.blink_enabled);
        }

        ESP_LOGI(TAG, "Reading blinkpin parameter from NVS ... ");
        err = nvs_get_i8(my_handle, "blinkpin", &aux);
        switch (err)
        {
        case ESP_OK:
            PrjCfg_dre.blink_io_num = (gpio_num_t)aux;
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.blink_io_num);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The blinkpin value is not initialized yet, using %d!", PrjCfg_dre.blink_io_num);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading blinkpin, using %d!", esp_err_to_name(err), PrjCfg_dre.blink_io_num);
        }

        ESP_LOGI(TAG, "Reading blinkval from NVS ... ");
        err = nvs_get_u8(my_handle, "blinkval", (uint8_t *)&PrjCfg_dre.blink_on_value);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.blink_on_value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The blinkval value is not initialized yet, using %d!", PrjCfg_dre.blink_on_value);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading blinkval, using %d!", esp_err_to_name(err), PrjCfg_dre.blink_on_value);
        }

        ESP_LOGI(TAG, "Reading blinkon parameter from NVS ... ");
        err = nvs_get_u32(my_handle, "blinkon", &PrjCfg_dre.blink_on_ms);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %lu", PrjCfg_dre.blink_on_ms);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The blinkon value is not initialized yet, using %lu!", PrjCfg_dre.blink_on_ms);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading blinkon, using %lu!", esp_err_to_name(err), PrjCfg_dre.blink_on_ms);
        }

        ESP_LOGI(TAG, "Reading blinkoff parameter from NVS ... ");
        err = nvs_get_u32(my_handle, "blinkoff", &PrjCfg_dre.blink_off_ms);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %lu", PrjCfg_dre.blink_off_ms);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The blinkoff value is not initialized yet, using %lu!", PrjCfg_dre.blink_off_ms);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading blinkoff, using %lu!", esp_err_to_name(err), PrjCfg_dre.blink_off_ms);
        }
#endif

#ifdef CONFIG_PORIS_ENABLE_HOVER
        ESP_LOGI(TAG, "Reading hover from NVS ... ");
        err = nvs_get_u8(my_handle, "hover", (uint8_t *)&PrjCfg_dre.hover_enabled);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.hover_enabled);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The hover value is not initialized yet, using %d!", PrjCfg_dre.hover_enabled);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading hover, using %d!", esp_err_to_name(err), PrjCfg_dre.hover_enabled);
        }

        ESP_LOGI(TAG, "Reading uart_hover_port parameter from NVS ... ");
        err = nvs_get_u8(my_handle, "uhport", (uint8_t *)&PrjCfg_dre.uart_hover_port);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_hover_port);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_hover_port value is not initialized yet, using %d!", PrjCfg_dre.uart_hover_port);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_hover_port, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_hover_port);
        }


        ESP_LOGI(TAG, "Reading uart_hover_rx_io_num parameter from NVS ... ");
        err = nvs_get_i8(my_handle, "uhrx", &aux);
        switch (err)
        {
        case ESP_OK:
            PrjCfg_dre.uart_hover_rx_io_num = (gpio_num_t)aux;
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_hover_rx_io_num);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_hover_rx_io_num value is not initialized yet, using %d!", PrjCfg_dre.uart_hover_rx_io_num);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_hover_rx_io_num, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_hover_rx_io_num);
        }

        ESP_LOGI(TAG, "Reading uart_hover_tx_io_num parameter from NVS ... ");
        err = nvs_get_i8(my_handle, "uhtx", &aux);
        switch (err)
        {
        case ESP_OK:
            PrjCfg_dre.uart_hover_tx_io_num = (gpio_num_t)aux;
            ESP_LOGI(TAG, "Done! %d", PrjCfg_dre.uart_hover_tx_io_num);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "The uart_hover_tx_io_num value is not initialized yet, using %d!", PrjCfg_dre.uart_hover_tx_io_num);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading uart_hover_tx_io_num, using %d!", esp_err_to_name(err), PrjCfg_dre.uart_hover_tx_io_num);
        }
#endif
        // Close
        nvs_close(my_handle);
    }

}
void PrjCfg_nvs_cfg_save(void)
{
    esp_err_t err;
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle_t my_handle;
    err = nvs_open("cfg", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        int8_t aux;
#ifdef CONFIG_PORIS_ENABLE_BLECENTRAL
        ESP_LOGI(TAG, "Writing central_role %d in NVS ... ", PrjCfg_dre.central_role);
        err = nvs_set_u8(my_handle, "rcentr", PrjCfg_dre.central_role);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_BLEPERIPHERAL
        ESP_LOGI(TAG, "Writing peripheral_role %d in NVS ... ", PrjCfg_dre.peripheral_role);
        err = nvs_set_u8(my_handle, "rperiph", PrjCfg_dre.peripheral_role);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing echo parameter %d in NVS ... ", PrjCfg_dre.echo);
        err = nvs_set_u8(my_handle, "echo", PrjCfg_dre.echo);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_OTA
        ESP_LOGI(TAG, "Writing skip_ota %d in NVS ... ", PrjCfg_dre.skip_ota);
        err = nvs_set_u8(my_handle, "otaskip", PrjCfg_dre.skip_ota);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTBRIDGE
        ESP_LOGI(TAG, "Writing uart_bridge %d in NVS ... ", PrjCfg_dre.uart_bridge);
        err = nvs_set_u8(my_handle, "uartbr", PrjCfg_dre.uart_bridge);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_UARTUSER
        ESP_LOGI(TAG, "Writing uart_user_port parameter %d in NVS ... ", PrjCfg_dre.uart_user_port);
        err = nvs_set_u8(my_handle, "uuport", PrjCfg_dre.uart_user_port);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_user_baudrate parameter %d in NVS ... ", PrjCfg_dre.uart_user_baudrate);
        err = nvs_set_i32(my_handle, "uubaud", PrjCfg_dre.uart_user_baudrate);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_user_stop_bits parameter %d in NVS ... ", PrjCfg_dre.uart_user_stop_bits);
        err = nvs_set_u8(my_handle, "uustopb", PrjCfg_dre.uart_user_stop_bits);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_user_rx_io_num parameter %d in NVS ... ", PrjCfg_dre.uart_user_rx_io_num);
        aux = (int8_t)PrjCfg_dre.uart_user_rx_io_num;
        err = nvs_set_i8(my_handle, "uurx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_user_tx_io_num parameter %d in NVS ... ", PrjCfg_dre.uart_user_tx_io_num);
        aux = (gpio_num_t)PrjCfg_dre.uart_user_tx_io_num;
        err = nvs_set_i8(my_handle, "uutx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }
#endif

#ifdef CONFIG_PORIS_ENABLE_UARTPERIPH
        ESP_LOGI(TAG, "Writing uart_periph_port parameter %d in NVS ... ", PrjCfg_dre.uart_periph_port);
        err = nvs_set_u8(my_handle, "upport", PrjCfg_dre.uart_periph_port);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_periph_baudrate parameter %d in NVS ... ", PrjCfg_dre.uart_periph_baudrate);
        err = nvs_set_i32(my_handle, "upbaud", PrjCfg_dre.uart_periph_baudrate);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_periph_stop_bits parameter %d in NVS ... ", PrjCfg_dre.uart_periph_stop_bits);
        err = nvs_set_u8(my_handle, "upstopb", PrjCfg_dre.uart_periph_stop_bits);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }


        ESP_LOGI(TAG, "Writing uart_periph_rx_io_num parameter %d in NVS ... ", PrjCfg_dre.uart_periph_rx_io_num);
        aux = (int8_t)PrjCfg_dre.uart_periph_rx_io_num;
        err = nvs_set_i8(my_handle, "uprx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }


        ESP_LOGI(TAG, "Writing uart_periph_tx_io_num parameter %d in NVS ... ", PrjCfg_dre.uart_periph_tx_io_num);
        aux = (gpio_num_t)PrjCfg_dre.uart_periph_tx_io_num;
        err = nvs_set_i8(my_handle, "uptx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_BLINK
        ESP_LOGI(TAG, "Writing blink %d in NVS ... ", PrjCfg_dre.blink_enabled);
        err = nvs_set_u8(my_handle, "blink", PrjCfg_dre.blink_enabled);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing blinkpin parameter %d in NVS ... ", PrjCfg_dre.blink_io_num);
        aux = (gpio_num_t)PrjCfg_dre.blink_io_num;
        err = nvs_set_i8(my_handle, "blinkpin", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }


        ESP_LOGI(TAG, "Writing blinkval %d in NVS ... ", PrjCfg_dre.blink_on_value);
        err = nvs_set_u8(my_handle, "blinkval", PrjCfg_dre.blink_on_value);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing blinkon parameter %lu in NVS ... ", PrjCfg_dre.blink_on_ms);
        err = nvs_set_u32(my_handle, "blinkon", PrjCfg_dre.blink_on_ms);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }


        ESP_LOGI(TAG, "Writing blinkoff parameter %lu in NVS ... ", PrjCfg_dre.blink_off_ms);
        err = nvs_set_u32(my_handle, "blinkoff", PrjCfg_dre.blink_off_ms);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }
#endif
#ifdef CONFIG_PORIS_ENABLE_HOVER
        ESP_LOGI(TAG, "Writing hover %d in NVS ... ", PrjCfg_dre.hover_enabled);
        err = nvs_set_u8(my_handle, "hover", PrjCfg_dre.hover_enabled);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_hover_port parameter %d in NVS ... ", PrjCfg_dre.uart_hover_port);
        err = nvs_set_u8(my_handle, "uhport", PrjCfg_dre.uart_hover_port);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_hover_tx_io_num parameter %d in NVS ... ", PrjCfg_dre.uart_hover_tx_io_num);
        aux = (gpio_num_t)PrjCfg_dre.uart_hover_tx_io_num;
        err = nvs_set_i8(my_handle, "uhtx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }

        ESP_LOGI(TAG, "Writing uart_hover_rx_io_num parameter %d in NVS ... ", PrjCfg_dre.uart_hover_rx_io_num);
        aux = (gpio_num_t)PrjCfg_dre.uart_hover_rx_io_num;
        err = nvs_set_i8(my_handle, "uhrx", aux);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Done!");
        }
#endif
        // Close
        nvs_close(my_handle);
    }
}

