#include <string.h>

#include "hw_regs.h"
#include "wheel_speed.h"

static WheelCalib wcal;
static u32        last_cnt[WHEEL_COUNT];
static u16        speed_x10[WHEEL_COUNT];

void wheel_init(const WheelCalib *calib)
{
    wcal = *calib; /* DEFECT(null_deref): calib is NULL when the NVM block is invalid (see main.c) */
    (void)memset(last_cnt, 0, sizeof(last_cnt));
    (void)memset(speed_x10, 0, sizeof(speed_x10));
}

void wheel_task_10ms(void)
{
    u32 w;

    for (w = 0U; w < WHEEL_COUNT; w++) {
        u32 cnt   = hw_enc[w].CNT;
        u32 delta = cnt - last_cnt[w]; /* SAFE(uint_wrap): counter wrap handled by modular subtraction */
        u32 mm_per_s;

        last_cnt[w] = cnt;
        /* delta ticks in 10 ms -> mm/s -> km/h * 10 */
        mm_per_s = (delta * wcal.circumference_mm * 100U) / wcal.ticks_per_rev; /* DEFECT(div_zero): ticks_per_rev == 0 in an erased calibration */
        speed_x10[w] = (u16)((mm_per_s * 36U) / 1000U);
    }
}

u16 wheel_speed_kmh_x10(WheelId wheel)
{
    if ((u32)wheel >= WHEEL_COUNT) {
        return 0U;
    }
    return speed_x10[wheel]; /* SAFE(out_of_bounds): checked above */
}

u16 vehicle_speed_kmh_x10(void)
{
    u32 sum = 0U;
    u32 w;

    for (w = 0U; w < WHEEL_COUNT; w++) {
        sum += speed_x10[w];
    }
    return (u16)(sum / WHEEL_COUNT);
}

float wheel_slip(WheelId wheel)
{
    float v_vehicle = (float)vehicle_speed_kmh_x10();
    float v_wheel   = (float)wheel_speed_kmh_x10(wheel);

    return (v_wheel - v_vehicle) / v_vehicle; /* DEFECT(div_zero): float division by zero at standstill */
}

u16 wheel_max_delta_x10(void)
{
    u16 lo = 0xFFFFU;
    u16 hi = 0U;
    u32 w;

    for (w = 0U; w < WHEEL_COUNT; w++) {
        if (speed_x10[w] < lo) {
            lo = speed_x10[w];
        }
        if (speed_x10[w] > hi) {
            hi = speed_x10[w];
        }
    }
    return (u16)(hi - lo); /* SAFE(uint_wrap): hi >= lo after the loop */
}
