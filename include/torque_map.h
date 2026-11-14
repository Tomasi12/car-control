#ifndef TORQUE_MAP_H
#define TORQUE_MAP_H

#include "std_types.h"

#define TQ_MAX_NM     250.0f
#define TQ_POINTS     4U
#define TQ_SHAPE_KIND 2U

float tq_thermal_factor(float temp_c);
float tq_thermal_factor_safe(float temp_c);
float tq_phase_current(float power_w);
float tq_vector_len(float x, float y);
s16   tq_to_can(float torque_nm);
s16   tq_to_can_safe(float torque_nm);
float tq_pedal_ratio(u8 pedal_pct);
float tq_pedal_ratio_safe(u8 pedal_pct);
float tq_lookup(u8 mode, u32 idx);
float tq_lookup_safe(u8 mode, u32 idx);
float tq_limit(float req);
float tq_checked(float req);
float tq_shape(u8 kind, float x);
float tq_shape_patched(float x);
float tq_request(u8 pedal_pct, u8 mode, s16 temp_c);
u32   tq_limit_hits(void);

#endif /* TORQUE_MAP_H */
