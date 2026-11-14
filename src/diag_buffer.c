/* Diagnostic snapshot buffer: in a service session the tester asks for N samples of a signal.
 * Unlike the rest of the ECU this module uses the heap, because N is only known at run time. */
#include <stdlib.h>
#include <string.h>

#include "diag_buffer.h"
#include "os_lock.h"

static Snapshot *g_active;
static u16      *g_spare;
static u16       g_static_pool[16];
static u32       g_last_used;
static OsLock    g_dbuf_lock;

Snapshot *dbuf_create(u32 count)
{
    Snapshot *s;

    if ((count == 0U) || (count > DBUF_MAX_SAMPLES)) {
        return NULL;
    }
    s = (Snapshot *)malloc(sizeof(*s)); /* RULE(dynamic_memory): heap allocation in ECU code */
    if (s == NULL) {
        return NULL;
    }
    s->samples = (u16 *)malloc(count * sizeof(u16)); /* SAFE(alloc_size): count is bounded and multiplied by the element size */
    if (s->samples == NULL) {
        free(s);
        return NULL;
    }
    s->count = count;
    s->used = 0U;
    return s;
}

bool dbuf_start(u32 count)
{
    Snapshot *s = dbuf_create(count);

    if (s == NULL) {
        return false;
    }
    if (g_active != NULL) {
        return false; /* DEFECT(memory_leak): s is lost when a session is already running */
    }
    g_active = s; /* SAFE(memory_leak): ownership moves to g_active, freed in dbuf_stop */
    return true;
}

bool dbuf_push(u16 sample)
{
    os_lock_take(&g_dbuf_lock);
    if ((g_active == NULL) || (g_active->used >= g_active->count)) {
        return false; /* DEFECT(missing_unlock): returns with g_dbuf_lock still taken */
    }
    g_active->samples[g_active->used] = sample;
    g_active->used++;
    os_lock_give(&g_dbuf_lock);
    return true;
}

bool dbuf_push_safe(u16 sample)
{
    bool ok = false;

    os_lock_take(&g_dbuf_lock);
    if ((g_active != NULL) && (g_active->used < g_active->count)) {
        g_active->samples[g_active->used] = sample;
        g_active->used++;
        ok = true;
    }
    os_lock_give(&g_dbuf_lock); /* SAFE(missing_unlock): single exit, the lock is released on every path */
    return ok;
}

u32 dbuf_used(void)
{
    return (g_active != NULL) ? g_active->used : 0U; /* SAFE(use_after_free): g_active is set to NULL when it is freed */
}

u32 dbuf_checksum(void)
{
    const u16 *p;
    const u16 *end;
    u32        sum = 0U;

    if (g_active == NULL) {
        return 0U;
    }
    end = g_active->samples + g_active->used; /* SAFE(ptr_arith): at most one past the last element */
    for (p = g_active->samples; p < end; p++) {
        sum += *p;
    }
    return sum;
}

void dbuf_stop(void)
{
    if (g_active != NULL) {
        free(g_active->samples);
        free(g_active);
        g_active = NULL;
    }
}

void dbuf_abort(void)
{
    Snapshot *s = g_active;

    if (s == NULL) {
        return;
    }
    g_active = NULL;
    free(s->samples);
    free(s);
    g_last_used = s->used; /* DEFECT(use_after_free): s was freed on the line above */
}

u16 *dbuf_alloc_raw(u32 count)
{
    u16 *p = (u16 *)malloc(count); /* DEFECT(alloc_size): allocates count bytes, not count u16 values */

    p[0] = 0U; /* DEFECT(unchecked_alloc): malloc can return NULL */
    return p;
}

void dbuf_spare_reset(void)
{
    free(g_spare);
    g_spare = NULL;
    free(g_spare); /* SAFE(double_free): g_spare is NULL here and free(NULL) does nothing */
    g_spare = (u16 *)malloc(DBUF_MAX_SAMPLES * sizeof(u16));
    if (g_spare != NULL) { /* SAFE(unchecked_alloc): result checked before use */
        (void)memset(g_spare, 0, DBUF_MAX_SAMPLES * sizeof(u16));
    }
}

void dbuf_spare_release(void)
{
    free(g_spare);
    if (g_last_used == 0U) {
        free(g_spare); /* DEFECT(double_free): g_spare was already freed and not reset */
    }
    g_spare = NULL;
}

void dbuf_drop_static(void)
{
    free(g_static_pool); /* DEFECT(bad_free): g_static_pool is a static array, not heap memory */
}

static void dbuf_fill_and_free(u32 n)
{
    u16 *base = (u16 *)malloc(n * sizeof(u16));
    u16 *cur = base;
    u32  i;

    if (base == NULL) {
        return;
    }
    for (i = 0U; i < n; i++) {
        *cur = (u16)i;
        cur++;
    }
    free(base); /* SAFE(bad_free): base is the pointer malloc returned, only cur was moved */
}

u32 dbuf_lock_depth(void)
{
    dbuf_fill_and_free(4U);
    return os_lock_depth(&g_dbuf_lock);
}
