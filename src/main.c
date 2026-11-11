/* car-control: a small embedded-style ECU (electric drive) that runs on the PC.
 *
 *   dummy_ecu.exe              normal 12 s drive cycle, no run-time errors
 *   dummy_ecu.exe --fault N    trigger planted defect N (see README.md)
 */
#include <stdio.h>
#include <string.h>

#include "adc.h"
#include "battery.h"
#include "can.h"
#include "crc.h"
#include "diag.h"
#include "filter.h"
#include "hw_regs.h"
#include "motor_ctrl.h"
#include "nvm.h"
#include "pid.h"
#include "ring_buffer.h"
#include "scheduler.h"
#include "util.h"
#include "watchdog.h"
#include "wheel_speed.h"

#define SIM_DURATION_MS  12000U
#define REPORT_PERIOD_MS 1000U
#define PEDAL_STEP_PCT   2

static u8         g_pedal_target;
static s16        g_pedal_pct;
static u8         g_mode;
static u32        g_crc_errors;
static RingBuffer g_config_log;
static MovingAvg  g_temp_avg;
static s32        g_batt_lp;
static s16        g_temp_c;
static u16        g_batt_mv;

/* ---------- CAN handlers ---------- */

static void on_pedal_frame(const CanFrame *f)
{
    bool crc_bad = !frame_crc_ok(f->data, f->dlc);

    diag_report(DTC_CAN_CRC, crc_bad, sched_now());
    if (crc_bad) {
        g_crc_errors++;
        return;
    }
    g_pedal_target = (u8)can_get_signal(f->data, 0U, 8U);
    if (g_pedal_target > 100U) {
        g_pedal_target = 100U;
    }
    g_mode = f->data[1];
}

static void on_config_frame(const CanFrame *f)
{
    (void)rb_push(&g_config_log, f->data[1]);
}

/* ---------- tasks ---------- */

static void task_adc_1ms(void)
{
    adc_sample_all();
}

static void task_control_10ms(void)
{
    CanFrame f;

    while (can_receive(&f)) {
        can_dispatch(&f);
    }
    wheel_task_10ms();

    g_pedal_pct = rate_limit(g_pedal_pct, (s16)g_pedal_target, PEDAL_STEP_PCT);
    g_temp_c = mavg_update(&g_temp_avg, adc_temp_celsius(adc_filtered(ADC_CH_MOTOR_TEMP)));
    motor_task_10ms((u8)g_pedal_pct, vehicle_speed_kmh_x10(), g_temp_c);

    diag_report(DTC_OVERTEMP, g_temp_c > 110, sched_now());
    diag_report(DTC_SPEED_DEV, wheel_max_delta_x10() > 50U, sched_now());
}

static void task_slow_100ms(void)
{
    CanFrame st;
    u16      mv = (u16)(adc_to_millivolts(adc_filtered(ADC_CH_BATTERY)) * 9U);

    g_batt_mv = (u16)lowpass_update(&g_batt_lp, (s32)mv, 2U);
    battery_task_100ms(g_batt_mv, (s16)adc_filtered(ADC_CH_CURRENT));
    diag_report(DTC_BATT_LOW, battery_low(), sched_now());

    (void)memset(&st, 0, sizeof(st));
    st.id = CAN_ID_STATUS;
    st.dlc = CAN_MAX_DLC;
    can_set_u16_le(st.data, 0U, vehicle_speed_kmh_x10());
    st.data[2] = battery_soc_pct();
    st.data[3] = (u8)motor_state();
    can_set_u16_le(st.data, 4U, (u16)motor_duty_permille());
    st.data[6] = g_mode;
    st.data[7] = crc8_j1850(st.data, 7U);
    can_send(&st);

    wdg_kick();
}

/* ---------- fault injection (planted defects) ---------- */

static const char *const fault_names[] = {
    "",
    "corrupt NVM -> wheel_init(NULL)         null_deref",
    "erased calibration -> ticks_per_rev = 0 div_zero",
    "task with period 0                      div_zero (modulo)",
    "CAN frame with DLC 15                   out_of_bounds (memcpy)",
    "freeze frame returns local address      stack_escape",
    "abs_s32(INT32_MIN)                      overflow",
    "battery voltage above table             out_of_bounds",
    "adc_raw(9)                              out_of_bounds",
    "more than 16 DTCs                       out_of_bounds + null_deref",
    "rate_limit(-30000, 30000)               overflow",
    "can_get_signal(length 32)               shift",
    "rb_raw(16)                              out_of_bounds",
    "battery_cell_mv(cells 0)                div_zero",
    "pid_update(dt 0)                        div_zero",
    "lowpass_update(shift 40)                shift",
};

#define FAULT_COUNT ((s32)ARRAY_LEN(fault_names) - 1)

static void list_faults(void)
{
    s32 i;

    printf("Fault injection modes (dummy_ecu.exe --fault N):\n");
    for (i = 1; i <= FAULT_COUNT; i++) {
        printf("  %2ld  %s\n", (long)i, fault_names[i]);
    }
}

static void run_late_fault(s32 fault)
{
    static const u8 frame[CAN_MAX_DLC] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U};
    volatile s32    big = INT32_MIN;
    Pid             p;
    u32             i;

    switch (fault) {
    case 5: {
        FreezeFrame *ff = diag_capture_freeze_frame(vehicle_speed_kmh_x10(), g_temp_c, g_batt_mv, (u8)motor_state());
        printf("freeze frame speed = %u\n", (unsigned)ff->speed_x10);
        break;
    }
    case 6:
        printf("abs_s32(INT32_MIN) = %ld\n", (long)abs_s32(big));
        break;
    case 7:
        battery_task_100ms(20000U, 0);
        printf("soc at 5000 mV/cell = %u %%\n", (unsigned)battery_soc_pct());
        break;
    case 8:
        printf("adc_raw(9) = %u\n", (unsigned)adc_raw(9U));
        break;
    case 9:
        for (i = 0U; i < 20U; i++) {
            diag_report((u16)(0x1000U + i), true, sched_now());
        }
        break;
    case 10:
        printf("rate_limit = %d\n", (int)rate_limit(-30000, 30000, 100));
        break;
    case 11:
        printf("signal = %lu\n", (unsigned long)can_get_signal(frame, 0U, 32U));
        break;
    case 12:
        printf("rb_raw(16) = %u\n", (unsigned)rb_raw(&g_config_log, RB_SIZE));
        break;
    case 13:
        printf("cell mV = %u\n", (unsigned)battery_cell_mv(15000U, 0U));
        break;
    case 14:
        pid_init(&p, 256, 2, 10, 0, 1000);
        printf("pid = %ld\n", (long)pid_update(&p, 100, 0, 0));
        break;
    case 15:
        printf("lowpass = %ld\n", (long)lowpass_update(&g_batt_lp, 1000, 40U));
        break;
    default:
        break;
    }
}

/* ---------- report ---------- */

static void print_status(u32 t)
{
    printf("%5lu ms | pedal %3d %% | speed %5.1f km/h (true %5.1f) | duty %4lu | %-6s | %3d C | %5u mV | SOC %3u %% | DTC %lu\n",
           (unsigned long)t, (int)g_pedal_pct, (double)vehicle_speed_kmh_x10() / 10.0, (double)hw_sim_true_speed_kmh(),
           (unsigned long)motor_duty_permille(), motor_state_name(motor_state()), (int)g_temp_c, (unsigned)g_batt_mv,
           (unsigned)battery_soc_pct(), (unsigned long)diag_active_count());
}

static void print_summary(void)
{
    u32 i;
    u8  v;
    s16 dev;

    printf("\n--- summary ---\n");
    for (i = 0U; i < sched_task_count(); i++) {
        const Task *t = sched_task(i);
        printf("task %-8s period %3u ms  runs %lu\n", t->name, (unsigned)t->period_ms, (unsigned long)t->runs);
    }
    printf("hyperperiod %lu ms, overruns %lu\n", (unsigned long)sched_hyperperiod(), (unsigned long)sched_overruns());
    printf("CAN rx %lu, tx %lu, CRC errors %lu, watchdog kicks %lu\n", (unsigned long)can_rx_count(),
           (unsigned long)CAN1->TX_COUNT, (unsigned long)g_crc_errors, (unsigned long)wdg_kicks());
    printf("last status frame CRC-16 0x%04X\n", (unsigned)crc16_ccitt((const u8 *)CAN1->TX.DATA, CAN_MAX_DLC));

    dev = abs_s16((s16)((s32)vehicle_speed_kmh_x10() - (s32)(hw_sim_true_speed_kmh() * 10.0f)));
    printf("speed error %d.%d km/h, median of FL/FR/RL %u, slip FL %.4f%s\n", (int)(dev / 10), (int)(dev % 10),
           (unsigned)median3((s16)wheel_speed_kmh_x10(WHEEL_FL), (s16)wheel_speed_kmh_x10(WHEEL_FR),
                             (s16)wheel_speed_kmh_x10(WHEEL_RL)),
           (double)wheel_slip(WHEEL_FL), float_equal(wheel_slip(WHEEL_FL), 0.0f) ? " (none)" : "");
    printf("motor current RMS-ish %u\n", (unsigned)sqrt_u32((u32)adc_filtered(ADC_CH_CURRENT) * adc_filtered(ADC_CH_CURRENT)));

    printf("config log (%lu):", (unsigned long)rb_count(&g_config_log));
    while (rb_pop(&g_config_log, &v)) {
        printf(" %u", (unsigned)v);
    }
    printf("\nDTCs:\n");
    for (i = 0U; i < DTC_MAX; i++) {
        const Dtc *d = diag_get(i);
        if ((d != NULL) && (d->code != 0U)) {
            printf("  0x%04X %s (first at %lu ms)\n", (unsigned)d->code,
                   ((d->status & 0x01U) != 0U) ? "CONFIRMED" : "passed", (unsigned long)d->first_tick);
        }
    }
}

/* ---------- main ---------- */

int main(int argc, char **argv)
{
    s32 fault = 0;
    u32 t;

    if ((argc >= 2) && (strcmp(argv[1], "--list") == 0)) {
        list_faults();
        return 0;
    }
    if ((argc >= 3) && (strcmp(argv[1], "--fault") == 0)) {
        bool ok;
        fault = parse_decimal(argv[2], &ok);
        if (!ok || (fault < 1) || (fault > FAULT_COUNT)) {
            list_faults();
            return 2;
        }
        printf("*** fault injection %ld: %s ***\n", (long)fault, fault_names[fault]);
    }

    crc16_init();
    nvm_init();
    if (fault == 1) {
        nvm_corrupt();
    }
    if (fault == 2) {
        static const WheelCalib erased = {0U, 1950U};
        (void)nvm_store_wheel_calib(&erased);
    }

    wheel_init(nvm_wheel_calib());
    adc_init();
    can_init();
    motor_init();
    diag_init();
    battery_init();
    wdg_init();
    rb_init(&g_config_log);
    mavg_init(&g_temp_avg);
    sched_init();

    can_register_handler(CAN_ID_PEDAL, on_pedal_frame);
    can_register_handler(CAN_ID_CONFIG, on_config_frame);

    (void)sched_add("adc", task_adc_1ms, 1U, 0U);
    (void)sched_add("control", task_control_10ms, 10U, 0U);
    (void)sched_add("slow", task_slow_100ms, 100U, 5U);
    if (fault == 3) {
        (void)sched_add("broken", task_adc_1ms, 0U, 0U);
    }

    hw_sim_step(0U);
    g_batt_lp = (s32)(adc_to_millivolts(adc_raw(ADC_CH_BATTERY)) * 9U);

    for (t = 1U; t <= SIM_DURATION_MS; t++) {
        if ((fault == 4) && (t == 1000U)) {
            hw_sim_bad_dlc = true;
        }
        hw_sim_step(t);
        sched_tick_isr();
        sched_run();
        if ((t % REPORT_PERIOD_MS) == 0U) {
            print_status(t);
        }
    }

    run_late_fault(fault);
    print_summary();
    return 0;
}
