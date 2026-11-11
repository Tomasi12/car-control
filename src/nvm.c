#include <string.h>

#include "crc.h"
#include "nvm.h"

static u8         eeprom[NVM_SIZE];
static WheelCalib wheel_ram;

void nvm_init(void)
{
    static const WheelCalib defaults = {1000U, 1950U};

    (void)memset(eeprom, 0xFF, sizeof(eeprom));
    (void)nvm_store_wheel_calib(&defaults);
}

bool nvm_read(u16 addr, u8 *dst, u16 len)
{
    if (addr >= NVM_SIZE) {
        return false;
    }
    (void)memcpy(dst, &eeprom[addr], len); /* DEFECT(out_of_bounds): addr + len is not checked against NVM_SIZE */
    return true;
}

bool nvm_write(u16 addr, const u8 *src, u16 len)
{
    if (((u32)addr + len) > NVM_SIZE) {
        return false;
    }
    (void)memcpy(&eeprom[addr], src, len); /* SAFE(out_of_bounds): full range checked above */
    return true;
}

u8 nvm_read_byte(u16 addr)
{
    return eeprom[addr % NVM_SIZE]; /* SAFE(out_of_bounds): index reduced modulo NVM_SIZE */
}

bool nvm_store_wheel_calib(const WheelCalib *calib)
{
    u8 raw[sizeof(WheelCalib) + 1U];

    (void)memcpy(raw, calib, sizeof(WheelCalib));
    raw[sizeof(WheelCalib)] = crc8_j1850(raw, sizeof(WheelCalib));
    return nvm_write(NVM_ADDR_WHEEL, raw, (u16)sizeof(raw));
}

const WheelCalib *nvm_wheel_calib(void)
{
    u8 raw[sizeof(WheelCalib) + 1U];

    nvm_read(NVM_ADDR_WHEEL, raw, (u16)sizeof(raw)); /* RULE(unchecked_return): return value of nvm_read is ignored */
    if (!frame_crc_ok(raw, sizeof(raw))) {
        return NULL;
    }
    (void)memcpy(&wheel_ram, raw, sizeof(wheel_ram));
    return &wheel_ram;
}

void nvm_corrupt(void)
{
    eeprom[NVM_ADDR_WHEEL] ^= 0xFFU;
}
