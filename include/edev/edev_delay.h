#ifndef EDEV_DELAY_H
#define EDEV_DELAY_H

#include <stdint.h>

/*
 * Drivers need to wait sometimes (e.g. after a chip reset). They cannot call
 * an MCU function directly, so the application registers one at startup.
 * If none is set, edev_delay_ms() does nothing (host tests need no delay).
 */
typedef void (*edev_delay_ms_fn)(uint32_t ms);

void edev_set_delay_fn(edev_delay_ms_fn fn);
void edev_delay_ms(uint32_t ms);

#endif /* EDEV_DELAY_H */