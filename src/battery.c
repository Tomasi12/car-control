#include "battery.h"
#include "util.h"

#define SOC_POINTS 6U

static const s16 soc_cell_mv[SOC_POINTS] = {3000, 3300, 3500, 3700, 3900, 4200};
static const s16 soc_table[SOC_POINTS]   = {0, 10, 30, 60, 85, 100};

static u8  soc_pct;
static s32 charge_mas; /* mA * s */

void battery_init(void)
{
    soc_pct = 0U;
    charge_mas = 0;
}

u16 battery_cell_mv(u16 pack_mv, u8 cells)
{
    return (u16)(pack_mv / cells); /* DEFECT(div_zero): cells == 0 */
}

void battery_task_100ms(u16 pack_mv, s16 current_ma)
{
    u16 cell = battery_cell_mv(pack_mv, BATTERY_CELLS);

    soc_pct = (u8)interp_table(soc_cell_mv, soc_table, SOC_POINTS, (s16)cell);
    charge_mas += current_ma / 10;
}

u8 battery_soc_pct(void)
{
    return soc_pct;
}

bool battery_low(void)
{
    return soc_pct < 10U;
}

s32 battery_charge_mas(void)
{
    return charge_mas;
}
