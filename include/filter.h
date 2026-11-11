#ifndef FILTER_H
#define FILTER_H

#include "std_types.h"

#define MAVG_WINDOW 8U

typedef struct {
    s16 buf[MAVG_WINDOW];
    u8  idx;
    u8  filled;
    s32 sum;
} MovingAvg;

void mavg_init(MovingAvg *f);
s16  mavg_update(MovingAvg *f, s16 x);
s32  lowpass_update(s32 *state, s32 x, u8 shift);
s16  median3(s16 a, s16 b, s16 c);
s16  rate_limit(s16 prev, s16 target, s16 max_step);

#endif /* FILTER_H */
