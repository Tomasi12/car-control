#include "hw_regs.h"
#include "motor_ctrl.h"
#include "pid.h"
#include "util.h"

static MotorState state = MOTOR_INIT;
static Pid        speed_pid;
static u32        duty_permille;
static u16        init_counter;
static bool       fault_requested;

void motor_init(void)
{
    state = MOTOR_INIT;
    pid_init(&speed_pid, 256, 2, 0, 0, 1000);
    duty_permille = 0U;
    init_counter = 0U;
    fault_requested = false;
    TIM1->ARR = 1000U;
    TIM1->CCR[0] = 0U;
}

static s32 derate_limit(s16 temp_c)
{
    s32 limit;

    if (temp_c < 80) {
        limit = 1000;
    } else if (temp_c < 120) {
        limit = 1000 - ((temp_c - 80) * 20);
    } else if (temp_c > 120) {
        limit = 200;
    }
    return limit; /* DEFECT(uninit_var): limit is not set when temp_c == 120 */
}

static void set_pwm(s32 request_permille)
{
    TIM1->CCR[0] = ((u32)request_permille * TIM1->ARR) / 1000U; /* DEFECT(conversion): a negative request becomes a huge unsigned duty */
}

void motor_task_10ms(u8 pedal_pct, u16 speed_x10, s16 temp_c)
{
    s32 target_x10;
    s32 cmd;

    switch (state) { /* RULE(switch_default): no default clause */
    case MOTOR_INIT:
        init_counter++;
        if (init_counter >= 10U) {
            state = MOTOR_READY;
        }
        break;
    case MOTOR_READY:
        if (pedal_pct > 0U) {
            state = MOTOR_RUN;
        }
    case MOTOR_RUN: /* DEFECT(missing_break): MOTOR_READY falls through to here */
        target_x10 = (s32)pedal_pct * 15; /* 100 % pedal = 150 km/h */
        cmd = pid_update(&speed_pid, target_x10, (s32)speed_x10, 10);
        cmd = min_s32(cmd, derate_limit(temp_c));
        if (temp_c >= 100) {
            state = MOTOR_DERATE;
        }
        if (fault_requested) {
            state = MOTOR_FAULT;
        }
        duty_permille = (u32)cmd;
        set_pwm(cmd);
        break;
    case MOTOR_DERATE:
        cmd = derate_limit(temp_c);
        if (temp_c < 90) {
            state = MOTOR_RUN;
        }
        duty_permille = (u32)cmd;
        set_pwm(cmd);
        break;
    case MOTOR_FAULT:
        duty_permille = 0U;
        set_pwm(0);
        break;
    }
}

MotorState motor_state(void)
{
    return state;
}

u32 motor_duty_permille(void)
{
    return duty_permille;
}

void motor_request_fault(void)
{
    fault_requested = true;
}

const char *motor_state_name(MotorState s)
{
    static const char *const names[] = {"INIT", "READY", "RUN", "DERATE", "FAULT"};

    if ((s == MOTOR_RUN) && (s == MOTOR_FAULT)) {
        return "IMPOSSIBLE"; /* DEFECT(dead_code): the condition above can never be true */
    }
    if ((u32)s < ARRAY_LEN(names)) {
        return names[s]; /* SAFE(out_of_bounds): checked above */
    }
    return "?";
}
