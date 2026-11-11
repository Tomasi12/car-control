#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "std_types.h"

typedef void (*TaskFn)(void);

typedef struct {
    const char *name;
    TaskFn      fn;
    u16         period_ms;
    u16         offset_ms;
    u32         runs;
} Task;

void        sched_init(void);
bool        sched_add(const char *name, TaskFn fn, u16 period_ms, u16 offset_ms);
void        sched_tick_isr(void); /* 1 ms timer interrupt */
void        sched_run(void);      /* main loop */
u32         sched_now(void);
u32         sched_hyperperiod(void);
u32         sched_task_count(void);
const Task *sched_task(u32 index);
u32         sched_overruns(void);

#endif /* SCHEDULER_H */
