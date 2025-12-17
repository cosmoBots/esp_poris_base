#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <PrjCfg.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    UIDemo_ret_error = -1,
    UIDemo_ret_ok    = 0
} UIDemo_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------

#pragma once
#include "lvgl.h"

extern lv_style_t style_header_bg;
extern lv_style_t style_btn_primary;
extern lv_style_t style_text_body;
extern lv_style_t style_text_invert;

void ui_styles_init(void);


// ------------------ END   Datatypes ------------------

// ------------------ BEGIN DRE ------------------
typedef struct {
    bool enabled;
    UIDemo_return_code_t last_return_code;

#include "UIDemo_netvar_types_fragment.h_"
} UIDemo_dre_t;

extern UIDemo_dre_t UIDemo_dre;
// ------------------ END   DRE ------------------

// ------------------ BEGIN Public API (MULTITASKING)--------------------
#if CONFIG_UIDEMO_USE_THREAD
/**
 *  Start background task that calls spin() every period.
 *  Idempotent. Returns error if task creation fails.
 */
UIDemo_return_code_t UIDemo_start(void);

/**
 *  Stop background task gracefully.
 *  Idempotent. Safe to call if not running.
 */
UIDemo_return_code_t UIDemo_stop(void);

/**
 *  Thread-safe clone of current DRE state.
 */
UIDemo_return_code_t UIDemo_get_dre_clone(UIDemo_dre_t *dst);

/**
 *  Change the periodic interval at runtime (ms).
 *  Clamped internamente a >= 10 ms.
 */
UIDemo_return_code_t UIDemo_set_period_ms(uint32_t period_ms);

/**
 *  Get current period in ms.
 */
uint32_t UIDemo_get_period_ms(void);

/**
 *  Execute a function wrapped with locks so you can access the DRE variables in thread-safe mode
*/
void UIDemo_execute_function_safemode(void (*callback)());


// ------------------ END   Public API (MULTITASKING)--------------------
#else
// ------------------ BEGIN Public API (SPIN)--------------------
/**
 *  Non-blocking step of this module (call it from your scheduler when
 *  CONFIG_UIDEMO_USE_THREAD = n).
 */
UIDemo_return_code_t UIDemo_spin(void);

// ------------------ END   Public API (SPIN)--------------------
#endif // CONFIG_UIDEMO_USE_THREAD

// ------------------ BEGIN Public API (COMMON)--------------------

/**
 *  Called at initialization time. Does minimal setup.
 */
UIDemo_return_code_t UIDemo_setup(void);

/**
 *  Enable/disable from user code (thread-safe if internal thread is enabled).
 */
UIDemo_return_code_t UIDemo_enable(void);
UIDemo_return_code_t UIDemo_disable(void);

// ------------------ BEGIN Public API (COMMON)--------------------

#ifdef __cplusplus
}
#endif
