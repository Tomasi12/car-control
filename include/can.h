#ifndef CAN_H
#define CAN_H

#include "std_types.h"

#define CAN_ID_PEDAL  0x120U
#define CAN_ID_CONFIG 0x130U
#define CAN_ID_STATUS 0x200U
#define CAN_MAX_DLC   8U

typedef struct {
    u32 id;
    u8  dlc;
    u8  data[CAN_MAX_DLC];
} CanFrame;

typedef void (*CanHandler)(const CanFrame *frame);

void can_init(void);
bool can_receive(CanFrame *out);
void can_send(const CanFrame *frame);
u32  can_get_signal(const u8 *data, u8 start_bit, u8 length);
void can_set_u16_le(u8 *data, u8 offset, u16 value);
void can_register_handler(u32 id, CanHandler handler);
void can_dispatch(const CanFrame *frame);
u32  can_rx_count(void);

#endif /* CAN_H */
