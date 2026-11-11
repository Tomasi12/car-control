#ifndef BATTERY_H
#define BATTERY_H

#include "std_types.h"

#define BATTERY_CELLS 4U

void battery_init(void);
void battery_task_100ms(u16 pack_mv, s16 current_ma);
u8   battery_soc_pct(void);
u16  battery_cell_mv(u16 pack_mv, u8 cells);
bool battery_low(void);
s32  battery_charge_mas(void);

#endif /* BATTERY_H */
