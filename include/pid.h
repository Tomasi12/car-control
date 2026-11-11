#ifndef PID_H
#define PID_H

#include "std_types.h"

/* Gains are Q8 fixed point: 256 == 1.0 */
typedef struct {
    s32 kp;
    s32 ki;
    s32 kd;
    s32 integral;
    s32 prev_error;
    s32 out_min;
    s32 out_max;
} Pid;

void pid_init(Pid *p, s32 kp, s32 ki, s32 kd, s32 out_min, s32 out_max);
s32  pid_update(Pid *p, s32 setpoint, s32 measured, s32 dt_ms);
void pid_reset(Pid *p);

#endif /* PID_H */
