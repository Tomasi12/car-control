#ifndef CRC_H
#define CRC_H

#include "std_types.h"

u8   crc8_j1850(const u8 *data, u32 len);
void crc16_init(void);
u16  crc16_ccitt(const u8 *data, u32 len);
bool frame_crc_ok(const u8 *frame, u32 len); /* last byte = CRC-8 of the others */

#endif /* CRC_H */
