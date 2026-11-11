/* Basic types used by all modules (embedded style: fixed-width only). */
#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#endif /* STD_TYPES_H */
