#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <PrjCfg.h>
#include <freertos/FreeRTOS.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    Relays_ret_error = -1,
    Relays_ret_ok    = 0
} Relays_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------
typedef enum {
    RELAY_STATE_OFF = 0,
    RELAY_STATE_ON  = 1,
} relay_state_t;

// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
#define RELAYS_MAX_COUNT 16

typedef struct {
    bool enabled;
    uint32_t count;
    int gpios[RELAYS_MAX_COUNT];
    relay_state_t states[RELAYS_MAX_COUNT];
    bool hw_init;
    Relays_return_code_t last_return_code;
} Relays_dre_t;

extern Relays_dre_t Relays_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_RELAYS_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
Relays_return_code_t Relays_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
Relays_return_code_t Relays_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
Relays_return_code_t Relays_get_dre_clone(Relays_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
Relays_return_code_t Relays_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t Relays_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void Relays_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_RELAYS_USE_THREAD = n).
 */
Relays_return_code_t Relays_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_RELAYS_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
Relays_return_code_t Relays_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
Relays_return_code_t Relays_enable(void);
Relays_return_code_t Relays_disable(void);

/**
 * Set relay state by index (0-based).
 */
Relays_return_code_t Relays_set_state(uint32_t relay_idx, relay_state_t state);

/**
 * Get configured relay count.
 */
uint32_t Relays_get_count(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
