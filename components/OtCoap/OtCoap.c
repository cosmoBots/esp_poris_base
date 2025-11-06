#include <PrjCfg.h>

#include "include/OtCoap.h"

#ifdef CONFIG_PORIS_ENABLE_OTCOAP
// Optional: code that depends on Kconfig switch

/**
 *  This is to be called on initialization
 */
OtCoap_return_code OtCoap_setup(void)
{
#error "Unimplemented"
    return OtCoap_ret_error;
}

/**
 * This is not blocking, an execution step for this module
 */
OtCoap_return_code OtCoap_spin(void)
{
#error "Unimplemented"
    return OtCoap_ret_error;
}

/**
 * In case you want to use a separate thread, use this function
 */
OtCoap_return_code OtCoap_start(void)
{
#error "Unimplemented"
    return OtCoap_ret_error;
}

/**
 * This is a thread-safe function to get a clone of the OtCoap_dre.
 */
OtCoap_return_code OtCoap_get_dre_clone(OtCoap_dre *OtCoap_dre_destination)
{
#error "Unimplemented"
    return OtCoap_ret_error;
}

/**
 * This is a thread-safe function to enable OtCoap.
 */
OtCoap_return_code OtCoap_enable(void)
{
#error "Unimplemented"
    return OtCoap_ret_error;
}

/**
 * This is a thread-safe function to disable OtCoap.
 */
OtCoap_return_code OtCoap_disable(void)
{
#error "Unimplemented"
    return OtCoap_ret_error;
}

#endif