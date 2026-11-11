#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "std_types.h"

void wdg_init(void);
void wdg_kick(void);
u32  wdg_kicks(void);

#endif /* WATCHDOG_H */
