#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum {
    PrjCfg_ret_error = -1,
    PrjCfg_ret_ok = 0
}PrjCfg_return_code;

typedef struct {
    bool enabled;
    PrjCfg_return_code last_return_code;
}PrjCfg_dre;

/**
 *  This is to be called on initialization
 */
PrjCfg_return_code PrjCfg_setup(void);

/**
 * This is not blocking, an execution step for this module
 */
PrjCfg_return_code PrjCfg_spin(void);

/**
 * In case you want to use a separate thread, use this function
 */
PrjCfg_return_code PrjCfg_start(void);

/**
 * This is a thread-safe function to get a clone of the PrjCfg_dre.
 */
PrjCfg_return_code PrjCfg_get_dre_clone(PrjCfg_dre *PrjCfg_dre_destination);

/**
 * This is a thread-safe function to enable PrjCfg.
 */
PrjCfg_return_code PrjCfg_enable(void);

/**
 * This is a thread-safe function to disable PrjCfg.
 */
PrjCfg_return_code PrjCfg_disable(void);


#ifdef __cplusplus
}
#endif