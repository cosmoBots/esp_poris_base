#include <stdio.h>
#include <PrjCfg.h>

#include "include/$$1.h"

#ifdef CONFIG_PORIS_ENABLE_$#1
// Optional: code that depends on Kconfig switch

/**
 *  This is to be called on initialization
 */
$$1_return_code $$1_setup(void)
{
#error "Unimplemented"
    return $$1_ret_error;
}

/**
 * This is not blocking, an execution step for this module
 */
$$1_return_code $$1_spin(void)
{
#error "Unimplemented"
    return $$1_ret_error;
}

/**
 * In case you want to use a separate thread, use this function
 */
$$1_return_code $$1_start(void)
{
#error "Unimplemented"
    return $$1_ret_error;
}

/**
 * This is a thread-safe function to get a clone of the $$1_dre.
 */
$$1_return_code $$1_get_dre_clone($$1_dre *$$1_dre_destination)
{
#error "Unimplemented"
    return $$1_ret_error;
}

/**
 * This is a thread-safe function to enable $$1.
 */
$$1_return_code $$1_enable(void)
{
#error "Unimplemented"
    return $$1_ret_error;
}

/**
 * This is a thread-safe function to disable $$1.
 */
$$1_return_code $$1_disable(void)
{
#error "Unimplemented"
    return $$1_ret_error;
}

#endif