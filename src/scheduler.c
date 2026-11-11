#include <string.h>

#include "scheduler.h"

#define SCHED_MAX_TASKS 8U

static Task tasks[SCHED_MAX_TASKS];
static u32  task_count;
static u32  now_ms;
static u32  pending_ticks; /* RULE(missing_volatile): shared with the timer ISR but not volatile */
static u32  overruns;

void sched_init(void)
{
    (void)memset(tasks, 0, sizeof(tasks));
    task_count = 0U;
    now_ms = 0U;
    pending_ticks = 0U;
    overruns = 0U;
}

bool sched_add(const char *name, TaskFn fn, u16 period_ms, u16 offset_ms)
{
    if (task_count >= SCHED_MAX_TASKS) {
        return false;
    }
    tasks[task_count].name = name; /* SAFE(out_of_bounds): task_count checked above */
    tasks[task_count].fn = fn;
    tasks[task_count].period_ms = period_ms;
    tasks[task_count].offset_ms = offset_ms;
    tasks[task_count].runs = 0U;
    task_count++;
    return true;
}

void sched_tick_isr(void)
{
    pending_ticks++; /* DEFECT(data_race): also modified by sched_run without disabling interrupts */
}

void sched_run(void)
{
    while (pending_ticks > 0U) {
        u32 i;

        pending_ticks--;
        now_ms++;
        for (i = 0U; i < task_count; i++) {
            Task *t = &tasks[i];
            if ((now_ms % t->period_ms) == t->offset_ms) { /* DEFECT(div_zero): sched_add accepts period_ms == 0 */
                t->fn();
                t->runs++;
            }
        }
        if (pending_ticks > 1U) {
            overruns++;
        }
    }
}

u32 sched_now(void)
{
    return now_ms;
}

static u32 gcd(u32 a, u32 b)
{
    if (b == 0U) {
        return a;
    }
    return gcd(b, a % b); /* RULE(recursion): function calls itself */
}

u32 sched_hyperperiod(void)
{
    u32 h = 1U;
    u32 i;

    for (i = 0U; i < task_count; i++) {
        u32 p = tasks[i].period_ms;
        if (p != 0U) {
            h = (h / gcd(h, p)) * p; /* SAFE(div_zero): gcd(h, p) >= 1 because p != 0 */
        }
    }
    return h;
}

u32 sched_task_count(void)
{
    return task_count;
}

const Task *sched_task(u32 index)
{
    return (index < task_count) ? &tasks[index] : NULL;
}

u32 sched_overruns(void)
{
    return overruns;
}
