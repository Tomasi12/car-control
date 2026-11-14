#include "os_lock.h"

void os_lock_take(OsLock *l)
{
    l->depth++;
}

void os_lock_give(OsLock *l)
{
    if (l->depth > 0U) {
        l->depth--;
    }
}

u32 os_lock_depth(const OsLock *l)
{
    return l->depth;
}
