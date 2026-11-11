#ifndef CRC8_H
#define CRC8_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t crc8(const uint8_t* data, size_t len);
int frame_checksum_ok(const uint8_t* frame, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* CRC8_H */
