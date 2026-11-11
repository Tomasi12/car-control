/* PC-only plant model: replaces the real hardware so the ECU code can run on Windows.
 * Not part of the "ECU" code under test. */
#include "can.h"
#include "crc.h"
#include "hw_regs.h"

AdcRegs hw_adc1;
CanRegs hw_can1;
TimRegs hw_tim1;
TimRegs hw_enc[ENCODER_COUNT];
WdgRegs hw_wdg;
bool    hw_sim_bad_dlc = false;

static float v_kmh;
static float temp_c = 25.0f;
static float enc_frac[ENCODER_COUNT];
static u8    alive_counter;

static u8 sim_pedal_pct(u32 t)
{
    if (t < 200U) {
        return 0U;
    }
    if (t < 3000U) {
        return 40U;
    }
    if (t < 6000U) {
        return 70U;
    }
    if (t < 10000U) {
        return 95U;
    }
    return 30U;
}

static void sim_put_frame(u32 mb, u32 id, const u8 *data, u32 dlc)
{
    u32 i;

    hw_can1.RX[mb].ID = id;
    hw_can1.RX[mb].DLC = dlc;
    for (i = 0U; i < 8U; i++) {
        hw_can1.RX[mb].DATA[i] = (i < dlc) ? data[i] : 0U;
    }
    hw_can1.RX_PENDING |= (1UL << mb);
}

static u32 to_adc(float value, float full_scale)
{
    float raw = (value / full_scale) * 4095.0f;
    if (raw < 0.0f) {
        raw = 0.0f;
    }
    if (raw > 4095.0f) {
        raw = 4095.0f;
    }
    return (u32)raw;
}

void hw_sim_step(u32 tick_ms)
{
    const float dt = 0.001f;
    float duty = 0.0f;
    u32   w;

    if (hw_tim1.ARR != 0U) {
        duty = (float)hw_tim1.CCR[0] / (float)hw_tim1.ARR;
    }
    if (duty > 1.0f) {
        duty = 1.0f;
    }

    /* vehicle: 100 % duty -> 150 km/h, time constant 5 s */
    v_kmh += ((duty * 150.0f) - v_kmh) * 0.2f * dt;
    /* motor temperature: 25 degC + 90 degC at full duty, time constant 2 s */
    temp_c += ((25.0f + (duty * 90.0f)) - temp_c) * 0.5f * dt;

    /* wheel encoders: 1000 ticks/rev, 1.95 m circumference -> v * 0.14245 ticks/ms */
    for (w = 0U; w < ENCODER_COUNT; w++) {
        u32 whole;
        enc_frac[w] += v_kmh * (1.0f + (0.002f * (float)w)) * 0.14245f;
        whole = (u32)enc_frac[w];
        hw_enc[w].CNT += whole;
        enc_frac[w] -= (float)whole;
    }

    /* ADC: throttle, battery (slow drain), temperature, current */
    hw_adc1.DR[0] = to_adc((float)sim_pedal_pct(tick_ms), 100.0f);
    hw_adc1.DR[1] = 2160U - (tick_ms / 150U) + (tick_ms % 3U);
    hw_adc1.DR[2] = to_adc(temp_c + 40.0f, 190.0f);
    hw_adc1.DR[3] = 2048U + (u32)(duty * 1000.0f) + (tick_ms % 7U);
    hw_adc1.SR |= ADC_SR_EOC;

    /* CAN: pedal frame every 10 ms, config frame every 500 ms */
    if ((tick_ms % 10U) == 5U) {
        u8 d[4];
        d[0] = sim_pedal_pct(tick_ms);
        d[1] = 1U; /* mode: normal */
        d[2] = alive_counter++;
        d[3] = crc8_j1850(d, 3U);
        sim_put_frame(0U, CAN_ID_PEDAL, d, hw_sim_bad_dlc ? 15U : 4U);
        hw_sim_bad_dlc = false;
    }
    if ((tick_ms % 500U) == 250U) {
        u8 d[2];
        d[0] = 0x01U;
        d[1] = (u8)(tick_ms / 500U);
        sim_put_frame(1U, CAN_ID_CONFIG, d, 2U);
    }

    /* watchdog */
    if (hw_wdg.KEY == WDG_KEY_RELOAD) {
        hw_wdg.KICKS++;
        hw_wdg.KEY = 0U;
    }
}

float hw_sim_true_speed_kmh(void)
{
    return v_kmh;
}
