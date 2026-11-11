#include <string.h>

#include "adc.h"
#include "hw_regs.h"

static u16 adc_hist[ADC_NUM_CHANNELS][ADC_HISTORY];
static u8  adc_pos;

void adc_init(void)
{
    (void)memset(adc_hist, 0, sizeof(adc_hist));
    adc_pos = 0U;
    ADC1->CR |= ADC_CR_START;
}

static u16 adc_wait_and_read(u8 hw_channel)
{
    while ((ADC1->SR & ADC_SR_EOC) == 0U) { /* DEFECT(non_terminating_loop): no timeout if the ADC never finishes */
    }
    return (u16)(ADC1->DR[hw_channel] & 0x0FFFU);
}

void adc_sample_all(void)
{
    u8 ch;

    for (ch = 0U; ch < ADC_NUM_CHANNELS; ch++) {
        adc_hist[ch][adc_pos] = adc_wait_and_read(ch);
    }
    ADC1->SR &= ~ADC_SR_EOC;
    adc_pos = (u8)((adc_pos + 1U) % ADC_HISTORY);
}

u16 adc_raw(u8 channel)
{
    return (u16)(ADC1->DR[channel] & 0x0FFFU); /* DEFECT(out_of_bounds): channel is not checked against ADC_HW_CHANNELS */
}

u16 adc_filtered(u8 channel)
{
    if (channel >= ADC_NUM_CHANNELS) {
        return 0U;
    }
    return adc_average(adc_hist[channel], ADC_HISTORY); /* SAFE(out_of_bounds): channel checked above */
}

u32 adc_to_millivolts(u16 raw)
{
    return ((u32)raw * 3300U) / 4095U; /* SAFE(overflow): 65535 * 3300 fits in u32 */
}

u16 adc_average(const u16 *samples, u32 n)
{
    u32 sum = 0U;
    u32 i;

    for (i = 0U; i < n; i++) {
        sum += samples[i];
    }
    return (u16)(sum / n); /* DEFECT(div_zero): n == 0 */
}

s16 adc_temp_celsius(u16 raw)
{
    /* NTC, linear approximation: 0..4095 -> -40..+150 degC */
    s32 t = (((s32)raw * 190) / 4095) - 40;
    return (s16)t;
}
