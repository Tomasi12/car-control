#include "crc8.h"

uint8_t crc8(const uint8_t* data, size_t len)
{
    uint8_t crc = 0xFFU;
    size_t i;
    for (i = 0U; i < len; i++) {
        uint8_t bit;
        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++) {
            crc = (uint8_t)(((crc & 0x80U) != 0U) ? ((crc << 1U) ^ 0x1DU) : (crc << 1U));
        }
    }
    return crc;
}

int frame_checksum_ok(const uint8_t* frame, size_t len)
{
    /* DEFECT: len == 0 gives frame[(size_t)-1] (out-of-bounds read). */
    return crc8(frame, len - 1U) == frame[len - 1U];
}
