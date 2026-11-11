#include "crc.h"

static u16  crc16_table[256];
static bool crc16_ready = false;

u8 crc8_j1850(const u8 *data, u32 len)
{
    u8  crc = 0xFFU;
    u32 i;

    for (i = 0U; i < len; i++) {
        u8 bit;
        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++) {
            if ((crc & 0x80U) != 0U) {
                crc = (u8)((u8)(crc << 1U) ^ 0x1DU);
            } else {
                crc = (u8)(crc << 1U);
            }
        }
    }
    return (u8)(crc ^ 0xFFU);
}

void crc16_init(void)
{
    u32 i;

    for (i = 0U; i < 256U; i++) {
        u16 c = (u16)(i << 8U);
        u8  bit;
        for (bit = 0U; bit < 8U; bit++) {
            if ((c & 0x8000U) != 0U) {
                c = (u16)((u16)(c << 1U) ^ 0x1021U);
            } else {
                c = (u16)(c << 1U);
            }
        }
        crc16_table[i] = c;
    }
    crc16_ready = true;
}

u16 crc16_ccitt(const u8 *data, u32 len)
{
    u16 crc = 0xFFFFU;
    u32 i;

    if (!crc16_ready) {
        crc16_init();
    }
    for (i = 0U; i < len; i++) {
        u8 idx = (u8)((crc >> 8U) ^ data[i]);
        crc = (u16)((u16)(crc << 8U) ^ crc16_table[idx]); /* SAFE(out_of_bounds): u8 index into a 256-entry table */
    }
    return crc;
}

bool frame_crc_ok(const u8 *frame, u32 len)
{
    return crc8_j1850(frame, len - 1U) == frame[len - 1U]; /* DEFECT(out_of_bounds): len == 0 reads frame[0xFFFFFFFF] */
}
