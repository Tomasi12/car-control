#include "util.h"

s32 sat_s32(s32 v, s32 lo, s32 hi)
{
    if (v < lo) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

s32 min_s32(s32 a, s32 b)
{
    return (a < b) ? a : b;
}

s16 abs_s16(s16 v)
{
    return (s16)((v < 0) ? -v : v); /* DEFECT(overflow): abs(INT16_MIN) does not fit in s16 */
}

s32 abs_s32(s32 v)
{
    return (v < 0) ? -v : v; /* DEFECT(overflow): -INT32_MIN is undefined */
}

s16 interp_table(const s16 *xs, const s16 *ys, u32 n, s16 x)
{
    u32 i = 0U;

    if (x <= xs[0]) {
        return ys[0];
    }
    while ((i < (n - 1U)) && (x > xs[i + 1U])) {
        i++;
    }
    return (s16)(ys[i] + (((s32)(ys[i + 1U] - ys[i]) * (x - xs[i])) / (xs[i + 1U] - xs[i]))); /* DEFECT(out_of_bounds): i + 1 == n when x > xs[n - 1] */
}

u16 sqrt_u32(u32 v)
{
    u32 res = 0U;
    u32 bit = 1UL << 30U;

    while (bit > v) {
        bit >>= 2U;
    }
    while (bit != 0U) {
        if (v >= (res + bit)) {
            v -= res + bit;
            res = (res >> 1U) + bit;
        } else {
            res >>= 1U;
        }
        bit >>= 2U;
    }
    return (u16)res; /* SAFE(conversion): sqrt of a u32 always fits in u16 */
}

bool float_equal(float a, float b)
{
    return a == b; /* RULE(float_equality): floats compared with == */
}

s32 parse_decimal(const char *s, bool *ok)
{
    s32  value = 0;
    bool neg = false;

    *ok = false;
    if (s == NULL) {
        goto done; /* RULE(goto): goto used */
    }
    if (*s == '-') {
        neg = true;
        s++;
    }
    if (*s == '\0') {
        goto done;
    }
    while (*s != '\0') {
        if ((*s < '0') || (*s > '9')) {
            goto done;
        }
        value = (value * 10) + (*s - '0'); /* DEFECT(overflow): long digit strings overflow s32 */
        s++;
    }
    *ok = true;
done:
    return neg ? -value : value;
}
