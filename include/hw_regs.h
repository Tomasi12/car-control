/* Memory-mapped peripherals.
 * On a real MCU these are fixed addresses, e.g. ((AdcRegs *)0x40012000UL).
 * On the PC they are plain variables updated by hw_sim.c. */
#ifndef HW_REGS_H
#define HW_REGS_H

#include "std_types.h"

#define ADC_HW_CHANNELS 8U
#define ADC_CR_START    (1UL << 0)
#define ADC_SR_EOC      (1UL << 1)

typedef struct {
    volatile u32 CR;
    volatile u32 SR;
    volatile u32 DR[ADC_HW_CHANNELS];
} AdcRegs;

#define CAN_RX_MAILBOXES 4U

typedef struct {
    volatile u32 ID;
    volatile u32 DLC;
    volatile u8  DATA[8];
} CanMailbox;

typedef struct {
    CanMailbox   RX[CAN_RX_MAILBOXES];
    CanMailbox   TX;
    volatile u32 RX_PENDING; /* bit n set: RX[n] holds a frame */
    volatile u32 TX_COUNT;
} CanRegs;

typedef struct {
    volatile u32 CNT;
    volatile u32 ARR;
    volatile u32 CCR[4];
} TimRegs;

#define WDG_KEY_RELOAD 0xAAAAU

typedef struct {
    volatile u32 KEY;
    volatile u32 KICKS;
} WdgRegs;

#define ENCODER_COUNT 4U

extern AdcRegs hw_adc1;
extern CanRegs hw_can1;
extern TimRegs hw_tim1;
extern TimRegs hw_enc[ENCODER_COUNT];
extern WdgRegs hw_wdg;

#define ADC1 (&hw_adc1)
#define CAN1 (&hw_can1)
#define TIM1 (&hw_tim1)
#define WDG  (&hw_wdg)

/* PC simulation of the plant (vehicle, sensors, CAN bus). */
void  hw_sim_step(u32 tick_ms);
float hw_sim_true_speed_kmh(void);
extern bool hw_sim_bad_dlc; /* fault injection: next pedal frame has DLC 15 */

#endif /* HW_REGS_H */
