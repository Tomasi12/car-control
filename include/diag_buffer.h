#ifndef DIAG_BUFFER_H
#define DIAG_BUFFER_H

#include "std_types.h"

#define DBUF_MAX_SAMPLES 256U

typedef struct {
    u16 *samples;
    u32  count;
    u32  used;
} Snapshot;

Snapshot *dbuf_create(u32 count);
bool      dbuf_start(u32 count);
bool      dbuf_push(u16 sample);
bool      dbuf_push_safe(u16 sample);
u32       dbuf_used(void);
u32       dbuf_checksum(void);
void      dbuf_stop(void);
void      dbuf_abort(void);
u16      *dbuf_alloc_raw(u32 count);
void      dbuf_spare_reset(void);
void      dbuf_spare_release(void);
void      dbuf_drop_static(void);
u32       dbuf_lock_depth(void);

#endif /* DIAG_BUFFER_H */
