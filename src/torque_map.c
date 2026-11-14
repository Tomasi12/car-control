/* Torque map: turns pedal, drive mode and motor temperature into a torque request (float math).
 * The result is sent on CAN in 0.01 Nm units. */
#include <math.h>

#include "torque_map.h"

#define TQ_R_PHASE_OHM  0.05f
#define TQ_TEMP_REF_C   90.0f
#define TQ_TEMP_SLOPE   0.9f
#define TQ_TEMP_CLAMP_C 150.0f
#define TQ_CAN_SCALE    100.0f

typedef float (*TqShaper)(float x);

static const float g_curve_normal[TQ_POINTS] = {0.0f, 80.0f, 160.0f, 220.0f};
static const u8    g_patch_area[16];
static u32         g_limit_hits;

float tq_thermal_factor(float temp_c)
{
    return expf((temp_c - TQ_TEMP_REF_C) * TQ_TEMP_SLOPE); /* DEFECT(float_overflow): above about 189 C (sensor fault) the exponent passes 88 and expf returns infinity */
}

float tq_thermal_factor_safe(float temp_c)
{
    if (temp_c > TQ_TEMP_CLAMP_C) {
        temp_c = TQ_TEMP_CLAMP_C;
    }
    return expf((temp_c - TQ_TEMP_REF_C) * TQ_TEMP_SLOPE); /* SAFE(float_overflow): exponent at most 54 after the clamp */
}

float tq_phase_current(float power_w)
{
    return sqrtf(power_w / TQ_R_PHASE_OHM); /* DEFECT(float_invalid): negative power while regenerating gives the root of a negative number (NaN) */
}

float tq_vector_len(float x, float y)
{
    return sqrtf((x * x) + (y * y)); /* SAFE(float_invalid): a sum of squares is never negative */
}

s16 tq_to_can(float torque_nm)
{
    return (s16)(torque_nm * TQ_CAN_SCALE); /* DEFECT(float_conversion): above 327.67 Nm the value does not fit in s16 */
}

s16 tq_to_can_safe(float torque_nm)
{
    if (torque_nm > TQ_MAX_NM) {
        torque_nm = TQ_MAX_NM;
    } else if (torque_nm < -TQ_MAX_NM) {
        torque_nm = -TQ_MAX_NM;
    } else if (isnan(torque_nm)) {
        torque_nm = 0.0f;
    }
    return (s16)(torque_nm * TQ_CAN_SCALE); /* SAFE(float_conversion): clamped to +-25000 before the cast */
}

float tq_pedal_ratio(u8 pedal_pct)
{
    float ratio = pedal_pct / 100U; /* DEFECT(integer_division_in_float): integer division, the ratio is 0 below 100 % */

    return ratio;
}

float tq_pedal_ratio_safe(u8 pedal_pct)
{
    return (float)pedal_pct / 100.0f; /* SAFE(integer_division_in_float): division is done in float */
}

float tq_lookup(u8 mode, u32 idx)
{
    const float *curve = g_curve_normal;

    if (idx >= TQ_POINTS) {
        idx = TQ_POINTS - 1U;
    }
    if (mode == 2U) {
        float sport_curve[TQ_POINTS] = {0.0f, 120.0f, 200.0f, 250.0f};
        curve = sport_curve;
    }
    return curve[idx]; /* DEFECT(dangling_ptr): sport_curve no longer exists after its block */
}

float tq_lookup_safe(u8 mode, u32 idx)
{
    const float *curve = g_curve_normal;

    if (idx >= TQ_POINTS) {
        idx = TQ_POINTS - 1U;
    }
    if (mode == 2U) {
        static const float sport_curve[TQ_POINTS] = {0.0f, 120.0f, 200.0f, 250.0f};
        curve = sport_curve;
    }
    return curve[idx]; /* SAFE(dangling_ptr): sport_curve is static and lives for the whole program */
}

float tq_limit(float req)
{
    if (req > TQ_MAX_NM) {
        return TQ_MAX_NM;
        g_limit_hits++; /* DEFECT(unreachable_code): statement after return never runs */
    }
    return req;
}

static void tq_fatal(void)
{
    for (;;) {
        /* wait for the watchdog reset */
    }
}

float tq_checked(float req)
{
    if (isnan(req)) {
        tq_fatal(); /* DEFECT(non_terminating_call): tq_fatal never returns */
    }
    return tq_limit(req);
}

static float shape_linear(float x)
{
    return x;
}

static float shape_soft(float x)
{
    return (x * x) / TQ_MAX_NM;
}

static const TqShaper g_shapers[TQ_SHAPE_KIND] = {shape_linear, shape_soft};

float tq_shape(u8 kind, float x)
{
    TqShaper f = shape_linear;

    if (kind < TQ_SHAPE_KIND) {
        f = g_shapers[kind];
    }
    return f(x); /* SAFE(bad_func_ptr): f always points to one of the two shaper functions */
}

float tq_shape_patched(float x)
{
    TqShaper patch = (TqShaper)(uintptr_t)g_patch_area;

    return patch(x); /* DEFECT(bad_func_ptr): patch points to a data array, not to a function */
}

float tq_request(u8 pedal_pct, u8 mode, s16 temp_c)
{
    float base;

    if (pedal_pct <= 255U) { /* RULE(invariant_condition): always true for a u8 */
        base = tq_lookup_safe(mode, ((u32)pedal_pct + 33U) / 34U);
    } else {
        base = 0.0f;
    }
    base = tq_shape(mode == 2U ? 0U : 1U, base) * tq_pedal_ratio_safe(pedal_pct);
    return tq_limit(base / (1.0f + tq_thermal_factor_safe((float)temp_c)));
}

u32 tq_limit_hits(void)
{
    return g_limit_hits;
}
