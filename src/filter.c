#include <string.h>

#include "filter.h"

void mavg_init(MovingAvg *f)
{
    (void)memset(f, 0, sizeof(*f));
}

s16 mavg_update(MovingAvg *f, s16 x)
{
    f->sum -= f->buf[f->idx];
    f->buf[f->idx] = x;
    f->sum += x;
    f->idx = (u8)((f->idx + 1U) % MAVG_WINDOW);
    if (f->filled < MAVG_WINDOW) {
        f->filled++;
    }
    return (s16)(f->sum / (s32)f->filled); /* SAFE(div_zero): filled >= 1 here */
}

s32 lowpass_update(s32 *state, s32 x, u8 shift)
{
    *state += (x - *state) >> shift; /* DEFECT(shift): shift >= 32 is undefined */
    return *state;
}

s16 median3(s16 a, s16 b, s16 c)
{
    if (a > b) {
        if (b > c) {
            return b;
        }
        return (a > c) ? c : a;
    }
    if (a > c) {
        return a;
    }
    return (b > c) ? c : b;
}

s16 rate_limit(s16 prev, s16 target, s16 max_step)
{
    s16 diff = (s16)(target - prev); /* DEFECT(overflow): target - prev does not fit in s16, e.g. 30000 - (-30000) */

    if (diff > max_step) {
        return (s16)(prev + max_step);
    }
    if (diff < -max_step) {
        return (s16)(prev - max_step);
    }
    return target;
}
