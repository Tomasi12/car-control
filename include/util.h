#ifndef UTIL_H
#define UTIL_H

#include "std_types.h"

s32  sat_s32(s32 v, s32 lo, s32 hi);
s32  min_s32(s32 a, s32 b);
s16  abs_s16(s16 v);
s32  abs_s32(s32 v);
s16  interp_table(const s16 *xs, const s16 *ys, u32 n, s16 x);
u16  sqrt_u32(u32 v);
bool float_equal(float a, float b);
s32  parse_decimal(const char *s, bool *ok);

#endif /* UTIL_H */
