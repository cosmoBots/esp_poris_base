#include <stdio.h>

#include "include/PrjCfg.h"

#ifdef CONFIG_PORIS_ENABLE_PRJCFG
// Optional: code that depends on Kconfig switch

/**
 *  This is to be called on initialization
 */
PrjCfg_return_code PrjCfg_setup(void)
{
    return PrjCfg_ret_ok;
}

/**
 * This is not blocking, an execution step for this module
 */
PrjCfg_return_code PrjCfg_spin(void)
{
    return PrjCfg_ret_ok;
}

/**
 * In case you want to use a separate thread, use this function
 */
PrjCfg_return_code PrjCfg_start(void)
{
    return PrjCfg_ret_ok;
}

/**
 * This is a thread-safe function to get a clone of the PrjCfg_dre.
 */
PrjCfg_return_code PrjCfg_get_dre_clone(PrjCfg_dre *PrjCfg_dre_destination)
{
    return PrjCfg_ret_ok;
}

/**
 * This is a thread-safe function to enable PrjCfg.
 */
PrjCfg_return_code PrjCfg_enable(void)
{
    return PrjCfg_ret_ok;
}

/**
 * This is a thread-safe function to disable PrjCfg.
 */
PrjCfg_return_code PrjCfg_disable(void)
{
    return PrjCfg_ret_ok;
}

#endif