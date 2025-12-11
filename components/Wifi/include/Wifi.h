#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    Wifi_ret_error = -1,
    Wifi_ret_ok    = 0
} Wifi_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------
#define WIFI_IP_STR_MAX_LEN 16
// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    Wifi_return_code_t last_return_code;
#include "Wifi_netvar_types_fragment.h_"

} Wifi_dre_t;

extern Wifi_dre_t Wifi_dre;
// ------------------ END   DRE ------------------


// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_WIFI_USE_THREAD = n).
 */
Wifi_return_code_t Wifi_spin(void);

// ------------------ END   Public API (SPIN)--------------------

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
Wifi_return_code_t Wifi_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
Wifi_return_code_t Wifi_enable(void);
Wifi_return_code_t Wifi_disable(void);

/**
 *  Obtén la IPv4 actual en formato texto. Devuelve error si no hay IP válida.
 */
Wifi_return_code_t Wifi_get_ipv4_str(char *buf, size_t len);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
