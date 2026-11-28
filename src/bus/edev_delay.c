#include "edev/edev_delay.h"

static edev_delay_ms_fn g_delay_fn;

void edev_set_delay_fn(edev_delay_ms_fn fn) { 
    
    g_delay_fn = fn; 
}

void edev_delay_ms(uint32_t ms) {
    
    if (g_delay_fn) g_delay_fn(ms);
}