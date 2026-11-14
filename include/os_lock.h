/* Minimal lock for data shared between scheduler tasks and the CAN service handler.
 * On the MCU take/give would disable and re-enable interrupts; on the PC they only count. */
#ifndef OS_LOCK_H
#define OS_LOCK_H

#include "std_types.h"

typedef struct {
    volatile u32 depth;
} OsLock;

void os_lock_take(OsLock *l);
void os_lock_give(OsLock *l);
u32  os_lock_depth(const OsLock *l);

#endif /* OS_LOCK_H */
