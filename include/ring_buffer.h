#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "std_types.h"

#define RB_SIZE 16U /* must be a power of two */
#define RB_MASK (RB_SIZE - 1U)

typedef struct {
    u8  data[RB_SIZE];
    u32 head;
    u32 tail;
} RingBuffer;

void rb_init(RingBuffer *rb);
bool rb_push(RingBuffer *rb, u8 value);
bool rb_pop(RingBuffer *rb, u8 *value);
u32  rb_count(const RingBuffer *rb);
u8   rb_peek(const RingBuffer *rb, u32 offset);
u8   rb_raw(const RingBuffer *rb, u32 index);

#endif /* RING_BUFFER_H */
