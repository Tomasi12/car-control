#ifndef MOTOR_CTRL_H
#define MOTOR_CTRL_H

#include "std_types.h"

typedef enum { MOTOR_INIT = 0, MOTOR_READY, MOTOR_RUN, MOTOR_DERATE, MOTOR_FAULT } MotorState;

void        motor_init(void);
void        motor_task_10ms(u8 pedal_pct, u16 speed_x10, s16 temp_c);
MotorState  motor_state(void);
u32         motor_duty_permille(void);
void        motor_request_fault(void);
const char *motor_state_name(MotorState s);

#endif /* MOTOR_CTRL_H */
