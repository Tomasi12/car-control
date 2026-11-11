#include <string.h>

#include "ring_buffer.h"

void rb_init(RingBuffer *rb)
{
    (void)memset(rb, 0, sizeof(*rb));
}

u32 rb_count(const RingBuffer *rb)
{
    return rb->head - rb->tail; /* SAFE(uint_wrap): modular arithmetic is intended */
}

bool rb_push(RingBuffer *rb, u8 value)
{
    if (rb_count(rb) >= RB_SIZE) {
        return false;
    }
    rb->data[rb->head & RB_MASK] = value; /* SAFE(out_of_bounds): index is masked */
    rb->head++;
    return true;
}

bool rb_pop(RingBuffer *rb, u8 *value)
{
    if (rb_count(rb) == 0U) {
        return false;
    }
    *value = rb->data[rb->tail & RB_MASK];
    rb->tail++;
    return true;
}

u8 rb_peek(const RingBuffer *rb, u32 offset)
{
    return rb->data[(rb->tail + offset) & RB_MASK]; /* SAFE(out_of_bounds): index is masked */
}

u8 rb_raw(const RingBuffer *rb, u32 index)
{
    if (index <= RB_SIZE) {
        return rb->data[index]; /* DEFECT(out_of_bounds): check should be index < RB_SIZE */
    }
    return 0U;
}
