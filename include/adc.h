#ifndef ADC_H
#define ADC_H

#include "std_types.h"

#define ADC_CH_THROTTLE   0U
#define ADC_CH_BATTERY    1U
#define ADC_CH_MOTOR_TEMP 2U
#define ADC_CH_CURRENT    3U
#define ADC_NUM_CHANNELS  4U /* channels used by the application */
#define ADC_HISTORY       8U

void adc_init(void);
void adc_sample_all(void);
u16  adc_raw(u8 channel);
u16  adc_filtered(u8 channel);
u32  adc_to_millivolts(u16 raw);
u16  adc_average(const u16 *samples, u32 n);
s16  adc_temp_celsius(u16 raw);

#endif /* ADC_H */
