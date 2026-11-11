#ifndef DIAG_H
#define DIAG_H

#include "std_types.h"

#define DTC_MAX 16U

#define DTC_OVERTEMP  0x0A01U
#define DTC_BATT_LOW  0x0B02U
#define DTC_CAN_CRC   0x0C03U
#define DTC_SPEED_DEV 0x0D04U

typedef struct {
    u16 code;
    u8  status;      /* bit 0: confirmed */
    u8  occurrences; /* debounce counter */
    u32 first_tick;
} Dtc;

typedef struct {
    u16 speed_x10;
    s16 temp_c;
    u16 battery_mv;
    u8  motor_state;
} FreezeFrame;

void         diag_init(void);
void         diag_clear_all(void);
void         diag_report(u16 code, bool failed, u32 tick);
u8           diag_debounce(u8 counter, bool failed);
u32          diag_active_count(void);
const Dtc   *diag_get(u32 index);
FreezeFrame *diag_capture_freeze_frame(u16 speed_x10, s16 temp_c, u16 battery_mv, u8 state);

#endif /* DIAG_H */
