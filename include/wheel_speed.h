#ifndef WHEEL_SPEED_H
#define WHEEL_SPEED_H

#include "std_types.h"

#define WHEEL_COUNT 4U

typedef enum { WHEEL_FL = 0, WHEEL_FR, WHEEL_RL, WHEEL_RR } WheelId;

typedef struct {
    u16 ticks_per_rev;
    u16 circumference_mm;
} WheelCalib;

void  wheel_init(const WheelCalib *calib);
void  wheel_task_10ms(void);
u16   wheel_speed_kmh_x10(WheelId wheel);
u16   vehicle_speed_kmh_x10(void);
float wheel_slip(WheelId wheel);
u16   wheel_max_delta_x10(void);

#endif /* WHEEL_SPEED_H */
