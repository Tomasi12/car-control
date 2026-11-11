#include "pid.h"
#include "util.h"

void pid_init(Pid *p, s32 kp, s32 ki, s32 kd, s32 out_min, s32 out_max)
{
    p->kp = kp;
    p->ki = ki;
    p->kd = kd;
    p->out_min = out_min;
    p->out_max = out_max;
    pid_reset(p);
}

void pid_reset(Pid *p)
{
    p->integral = 0;
    p->prev_error = 0;
}

s32 pid_update(Pid *p, s32 setpoint, s32 measured, s32 dt_ms)
{
    s32 error = setpoint - measured;
    s32 deriv;
    s32 out;

    p->integral += error * p->ki; /* DEFECT(overflow): no anti-windup, the integral grows without limit */
    deriv = ((error - p->prev_error) * 1000) / dt_ms; /* DEFECT(div_zero): dt_ms == 0 */
    p->prev_error = error;

    out = ((p->kp * error) + p->integral + (p->kd * deriv)) / 256;
    return sat_s32(out, p->out_min, p->out_max);
}
