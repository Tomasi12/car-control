#include <string.h>

#include "can.h"
#include "hw_regs.h"

#define CAN_BASE_ID  0x100U
#define CAN_HANDLERS 0x40U

static CanHandler can_handlers[CAN_HANDLERS];
static u32        rx_count;

void can_init(void)
{
    (void)memset(can_handlers, 0, sizeof(can_handlers));
    rx_count = 0U;
    CAN1->RX_PENDING = 0U;
}

bool can_receive(CanFrame *out)
{
    u32 mb;

    for (mb = 0U; mb < CAN_RX_MAILBOXES; mb++) {
        if ((CAN1->RX_PENDING & (1UL << mb)) != 0U) {
            const CanMailbox *box = &CAN1->RX[mb];
            out->id  = box->ID;
            out->dlc = (u8)box->DLC;
            (void)memcpy(out->data, (const void *)box->DATA, out->dlc); /* DEFECT(out_of_bounds): DLC from the bus can be up to 15, buffer is 8 */
            CAN1->RX_PENDING &= ~(1UL << mb);
            rx_count++;
            return true;
        }
    }
    return false;
}

void can_send(const CanFrame *frame)
{
    u8 i;

    CAN1->TX.ID  = frame->id;
    CAN1->TX.DLC = frame->dlc;
    for (i = 0U; (i < frame->dlc) && (i < CAN_MAX_DLC); i++) {
        CAN1->TX.DATA[i] = frame->data[i]; /* SAFE(out_of_bounds): bounded by CAN_MAX_DLC */
    }
    CAN1->TX_COUNT++;
}

u32 can_get_signal(const u8 *data, u8 start_bit, u8 length)
{
    u64 raw = 0U;
    u8  i;

    for (i = 0U; i < CAN_MAX_DLC; i++) {
        raw |= ((u64)data[i]) << (8U * i);
    }
    return (u32)((raw >> start_bit) & ((1UL << length) - 1UL)); /* DEFECT(shift): length == 32 shifts a 32-bit value by 32 */
}

void can_set_u16_le(u8 *data, u8 offset, u16 value)
{
    data[offset] = (u8)(value & 0xFFU);
    data[offset + 1U] = (u8)(value >> 8U); /* DEFECT(out_of_bounds): offset 7 writes data[8] */
}

void can_register_handler(u32 id, CanHandler handler)
{
    if ((id >= CAN_BASE_ID) && ((id - CAN_BASE_ID) < CAN_HANDLERS)) {
        can_handlers[id - CAN_BASE_ID] = handler; /* SAFE(out_of_bounds): range checked above */
    }
}

void can_dispatch(const CanFrame *frame)
{
    CanHandler h = can_handlers[frame->id - CAN_BASE_ID]; /* DEFECT(out_of_bounds): ids outside 0x100..0x13F are not checked */
    if (h != NULL) {
        h(frame);
    }
}

u32 can_rx_count(void)
{
    return rx_count;
}
