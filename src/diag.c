#include <string.h>

#include "diag.h"

static Dtc dtc_table[DTC_MAX];
static u32 dtc_count;

void diag_init(void)
{
    diag_clear_all();
}

void diag_clear_all(void)
{
    Dtc *p = dtc_table;

    (void)memset(p, 0, sizeof(p)); /* DEFECT(wrong_sizeof): clears sizeof(pointer) bytes, not the whole table */
    dtc_count = 0U;
}

u8 diag_debounce(u8 counter, bool failed)
{
    if (failed) {
        counter++; /* DEFECT(uint_wrap): 255 + 1 wraps to 0 and the fault "heals" */
    } else if (counter > 0U) {
        counter--;
    }
    return counter;
}

static Dtc *find_or_alloc(u16 code)
{
    u32 i;

    for (i = 0U; i <= DTC_MAX; i++) {
        if (dtc_table[i].code == code) { /* DEFECT(out_of_bounds): loop reaches i == DTC_MAX */
            return &dtc_table[i];
        }
        if (dtc_table[i].code == 0U) {
            dtc_table[i].code = code;
            dtc_count++;
            return &dtc_table[i];
        }
    }
    return NULL;
}

void diag_report(u16 code, bool failed, u32 tick)
{
    Dtc *d = find_or_alloc(code);

    d->occurrences = diag_debounce(d->occurrences, failed); /* DEFECT(null_deref): find_or_alloc returns NULL when the table is full */
    if (d->occurrences >= 3U) {
        d->status |= 0x01U;
        if (d->first_tick == 0U) {
            d->first_tick = tick;
        }
    } else {
        d->status &= (u8)~0x01U;
    }
}

u32 diag_active_count(void)
{
    u32 n = 0U;
    u32 i;

    for (i = 0U; i < dtc_count; i++) {
        if ((dtc_table[i].status & 0x01U) != 0U) {
            n++;
        }
    }
    return n;
}

const Dtc *diag_get(u32 index)
{
    if (index < DTC_MAX) {
        return &dtc_table[index]; /* SAFE(out_of_bounds): checked above */
    }
    return NULL;
}

FreezeFrame *diag_capture_freeze_frame(u16 speed_x10, s16 temp_c, u16 battery_mv, u8 state)
{
    FreezeFrame ff;

    ff.speed_x10 = speed_x10;
    ff.temp_c = temp_c;
    ff.battery_mv = battery_mv;
    ff.motor_state = state;
    return &ff; /* DEFECT(stack_escape): returns the address of a local variable */
}
