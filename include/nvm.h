#ifndef NVM_H
#define NVM_H

#include "std_types.h"
#include "wheel_speed.h"

#define NVM_SIZE       256U
#define NVM_ADDR_WHEEL 0U

void              nvm_init(void);
bool              nvm_read(u16 addr, u8 *dst, u16 len);
bool              nvm_write(u16 addr, const u8 *src, u16 len);
u8                nvm_read_byte(u16 addr);
const WheelCalib *nvm_wheel_calib(void); /* NULL if the block CRC is wrong */
bool              nvm_store_wheel_calib(const WheelCalib *calib);
void              nvm_corrupt(void);

#endif /* NVM_H */
