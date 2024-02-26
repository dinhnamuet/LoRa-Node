#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__
#include <stdint.h>
void watchdog_init(void);
void ping_to_wdt(void);
#endif
