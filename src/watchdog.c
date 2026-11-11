#include "hw_regs.h"
#include "watchdog.h"

void wdg_init(void)
{
    WDG->KEY = 0U;
    WDG->KICKS = 0U;
}

void wdg_kick(void)
{
    WDG->KEY = WDG_KEY_RELOAD;
}

u32 wdg_kicks(void)
{
    return WDG->KICKS;
}
