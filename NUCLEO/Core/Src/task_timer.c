#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#include "stm32g4xx_hal_uart.h"
#include"stm32g4xx_hal_def.h"
#include "timer.h"
#include "type.h"
#include "state.h"
#include<stdio.h>
#include<string.h>
#include<timers.h>

static ais140_uchar     sleep_timer_started = 0;
static ais140_uchar     halt_timer_started = 0;
ais140_uchar     config_change_flag = 0;

 int wdt_counter = 0;
unsigned int plat_second_count = 0;

// Timer mode constants
#define TIMER_ONE_SHOT 0
#define TIMER_PERIODIC 1

TIMER_HANDLE timer_array[MAX_TIMER_COUNT];

// -------------------------------------------------
// Internal callback wrapper (FreeRTOS → user callback)
// -------------------------------------------------
static void ais140_timer_callback_function(TimerHandle_t xTimer)
{
    TIMER_HANDLE *p_timer_hndl = (TIMER_HANDLE *)pvTimerGetTimerID(xTimer);

    if (p_timer_hndl == NULL || p_timer_hndl->p_callback == NULL)
        return;

    // Call user-defined callback
    p_timer_hndl->p_callback(p_timer_hndl->p_context);
}

// -------------------------------------------------
// Initialize timer array
// -------------------------------------------------
void itracer_timer_init(void)
{
    for(int i=0; i<MAX_TIMER_COUNT; i++) {
        memset(&timer_array[i],0x00,sizeof(TIMER_HANDLE));
    }
    UART_Printf("Timer system initialized.\r\n");
}

// -------------------------------------------------
// Create a new software timer
// -------------------------------------------------
int itracer_create_timer(int timer_id, ITRACER_TIMER_CALLBACK p_callback, void *p_context)
{
    if (timer_id >= MAX_TIMER_COUNT)
    {
        UART_Printf("TIMER-LOG: Invalid timer ID %d\r\n", timer_id);
        return 1;
    }

    TIMER_HANDLE *p_timer_hndl = &timer_array[timer_id];

    // Create FreeRTOS timer (dummy period for now)
    p_timer_hndl->timer_handle = xTimerCreate(
        "AIS140_Timer",
        pdMS_TO_TICKS(1000),
        pdFALSE,
        (void *)p_timer_hndl,
        ais140_timer_callback_function
    );

    if (p_timer_hndl->timer_handle == NULL)
    {
        UART_Printf("TIMER-LOG: Failed to create timer\r\n");
        return 2;
    }

    p_timer_hndl->p_callback = p_callback;
    p_timer_hndl->p_context = p_context;

    UART_Printf("TIMER-LOG: Created timer ID %d\r\n", timer_id);
    return 0;
}

// -------------------------------------------------
// Start the timer
// -------------------------------------------------
int itracer_start_timer(int timer_id, int m_sec, int mode)
{
    if (timer_id >= MAX_TIMER_COUNT)
        return 1;

    TIMER_HANDLE *p_timer_hndl = &timer_array[timer_id];
    if (p_timer_hndl->timer_handle == NULL)
        return 2;

    // Change timer period
    xTimerChangePeriod(p_timer_hndl->timer_handle, pdMS_TO_TICKS(m_sec), 0);

    // Make it periodic or one-shot
    if (mode == TIMER_PERIODIC)
    {
        vTimerSetReloadMode(p_timer_hndl->timer_handle, pdTRUE);
    }
    else
    {
        vTimerSetReloadMode(p_timer_hndl->timer_handle, pdFALSE);
    }

    if (xTimerStart(p_timer_hndl->timer_handle, 0) != pdPASS)
    {
        UART_Printf("TIMER-LOG: Failed to start timer\r\n");
        return 3;
    }

    UART_Printf("TIMER-LOG: Started timer ID %d (%d ms)\r\n", timer_id, m_sec);
    return 0;
}

// -------------------------------------------------
// Stop the timer
// -------------------------------------------------
int itracer_stop_timer(int timer_id)
{
    if (timer_id >= MAX_TIMER_COUNT)
        return 1;

    TIMER_HANDLE *p_timer_hndl = &timer_array[timer_id];
    if (p_timer_hndl->timer_handle == NULL)
        return 2;

    xTimerStop(p_timer_hndl->timer_handle, 0);
    UART_Printf("TIMER-LOG: Stopped timer ID %d\r\n", timer_id);
    return 0;
}


// --------------------------------------------------------------------
// Start Sleep Timer
// --------------------------------------------------------------------
int itracer_start_sleep_timer(void)
{
    int ret;
    uint16_t sleep_time;
    char buffer_log[100];

    if (sleep_timer_started == 1)
        return 1;

    // sleep_time = ais140_get_sleep_time();
    // if (sleep_time < 5)
    // {
    //     sleep_time = DEFAULT_SLEEP_TIME;
    //     ais140_set_sleep_time(DEFAULT_SLEEP_TIME);
    //     config_change_flag = 1;
    // }

    // if (itracer_get_dbg_flag() & ICOM_LOG_INFO)
    // {
    //     sUART_Printf(buffer_log, "TIMER-LOG: Sleep Time (%d)\r\n", sleep_time);
    //     itracer_dbg_log(ICOM_LOG_INFO, buffer_log);
    // // }

    ret = itracer_start_timer(AIS140_SLEEP_TIMER, sleep_time * 1000, TIMER_ONE_SHOT);
    if (ret == 0)
        sleep_timer_started = 1;

    return ret;
}

// --------------------------------------------------------------------
// Start Halt Timer
// --------------------------------------------------------------------
int itracer_start_halt_timer(void)
{
    int ret;
    uint16_t halt_time;
    char buffer_log[100];

    if (halt_timer_started == 1)
        return 1;

    // halt_time = ais140_get_halt_time();
    // if (halt_time < 5)
    // {
    //     halt_time = DEFAULT_HALT_TIME;
    //     ais140_set_halt_time(DEFAULT_HALT_TIME);
    //     config_change_flag = 1;
    // }

    // if (itracer_get_dbg_flag() & ICOM_LOG_INFO)
    // {
    //     sUART_Printf(buffer_log, "TIMER-LOG: Halt Time (%d)\r\n", halt_time);
    //     itracer_dbg_log(ICOM_LOG_INFO, buffer_log);
    // }

    ret = itracer_start_timer(AIS140_HALT_TIMER, halt_time * 1000, TIMER_ONE_SHOT);
    if (ret == 0)
        halt_timer_started = 1;

    return ret;
}

// --------------------------------------------------------------------
// Check if HALT Timer is Running
// --------------------------------------------------------------------
int ais140_is_halt_timer_running(void)
{
    return halt_timer_started;
}

// --------------------------------------------------------------------
// Stop Sleep Timer
// --------------------------------------------------------------------
int itracer_stop_sleep_timer(void)
{
    int ret;

    if (sleep_timer_started == 0)
        return 1;

    ret = itracer_stop_timer(AIS140_SLEEP_TIMER);
    if (ret == 0)
        sleep_timer_started = 0;

    return ret;
}

// --------------------------------------------------------------------
// Stop Halt Timer
// --------------------------------------------------------------------
int itracer_stop_halt_timer(void)
{
    int ret;

    if (halt_timer_started == 0)
        return 1;

    ret = itracer_stop_timer(AIS140_HALT_TIMER);
    if (ret == 0)
        halt_timer_started = 0;

    // if (itracer_get_dbg_flag() & ICOM_LOG_INFO)
        UART_Printf( "TIMER-LOG: Stopping HALT-TIMER\r\n");

    return ret;
}



// -------------------------------------------------------------
//  Sleep Timer Callback
// -------------------------------------------------------------
static void itracer_platmgr_sleep_callback(void *param)
{
    // Debug log
    // itracer_dbg_log(ICOM_LOG_INFO, "Veh-Mode: Sleep\r\n");

    ais140_set_vehicle_mode(VEHICLE_MODE_SLEEP);
    itracer_stop_sleep_timer();             // Stop Sleep timer once fired
    ais140_update_data_transfer_timer();    // Notify other modules
}

// -------------------------------------------------------------
//  Halt Timer Callback
// -------------------------------------------------------------
static void itracer_platmgr_halt_callback(void *param)
{
    /*
     * When the Halt timer fires:
     *  - Move from Motion → Halt mode
     *  - Start Sleep Timer if CDAC certification is active
     */
    // itracer_dbg_log(ICOM_LOG_INFO, "Veh-Mode: Halt\r\n");

    ais140_set_vehicle_mode(VEHICLE_MODE_HALT);
    itracer_stop_halt_timer();

    // if (ais140_get_state_flag(CDAC_CERTIFICATION) == CDAC_CERTIFICATION)
    // {
        itracer_start_sleep_timer();
    // }

    ais140_update_data_transfer_timer();
}

// -------------------------------------------------------------
//  1-Second Platform Timer Callback
// -------------------------------------------------------------
static void itracer_platmgr_1sec_callback(void *param)
{
    // Update vehicle mode every second
    ais140_update_vehicle_mode();

    // You can place periodic platform monitoring logic here
}

// -------------------------------------------------------------
//  Create & Start 1-Second Platform Timer
// -------------------------------------------------------------
static int itracer_platform_1_sec_timer_create(void)
{
    ais140_update_vehicle_mode(); // Initialize before first run

    itracer_create_timer(
        AIS140_ONE_SEC_TIMER,
        itracer_platmgr_1sec_callback,
        NULL
    );

    itracer_start_timer(
        AIS140_ONE_SEC_TIMER,
        AIS140_DEFAULT_DIGITAL_PORT_POLLING_FREQ,  // usually 1000ms
        TIMER_PERIODIC
    );

    return 0;
}

// -------------------------------------------------------------
//  Main 1-Second Timer Callback
// -------------------------------------------------------------
static void itracer_main_one_sec_callback(void *param)
{
    wdt_counter++;

    if (wdt_counter > 10)
    {
        // ais140_send_message(
        //     vts_task_main,
        //     VTS_MSG_ID_ONE_SEC_POLL,
        //     0,
        //     0
        // );
        wdt_counter = 0;
    }

    // If you want to pulse a watchdog pin periodically:
    // ais140_proces_wdt_pulse();
}

// -------------------------------------------------------------
//  Create & Start Main 1-Second Timer
// -------------------------------------------------------------
int itracer_main_one_sec_timer_create(void)
{
    itracer_create_timer(
        AIS140_MAIN_TASK_ONESEC_TIMER_ID,
        itracer_main_one_sec_callback,
        NULL
    );

    itracer_start_timer(
        AIS140_MAIN_TASK_ONESEC_TIMER_ID,
        100,   // 100 ms period
        TIMER_PERIODIC
    );

    return 0;
}
static int itracer_platform_60_sec_timer_create(void)
{
    // Update vehicle mode before starting timers
    ais140_update_vehicle_mode();

    // Create SLEEP timer
    itracer_create_timer(AIS140_SLEEP_TIMER,
                         itracer_platmgr_sleep_callback,
                         NULL);

    // Create HALT timer
    itracer_create_timer(AIS140_HALT_TIMER,
                         itracer_platmgr_halt_callback,
                         NULL);

    itracer_start_halt_timer();

    UART_Printf("TIMER-LOG: Platform 60-sec timers created and started\r\n");
    return 0;
}
static void itracer_gprs_timer_callback (void* param)
{
    // if(itracer_get_dbg_flag() & ICOM_LOG_INFO){
        UART_Printf( " TIMER-LOG : GPRS Timer fired \r\n");
    // }
    itracer_stop_timer(AIS140_GPRS_TIMER_ID);
    itracer_start_timer(AIS140_GPRS_TIMER_ID, AIS140_GPRS_TIMER_TIMEOUT_FREQ, 1);
    // ais140_send_message( vts_task_gprs, AIS140_MSG_ID_GPRS_ONE_SEC_CMD, 0, 0);

}

int itracer_gprs_timer_create()
{

    itracer_create_timer( AIS140_GPRS_TIMER_ID,
                          itracer_gprs_timer_callback, NULL);

    itracer_start_timer(AIS140_GPRS_TIMER_ID, AIS140_GPRS_TIMER_TIMEOUT_FREQ, 1);

    return (0);
}
static void itracer_adc_1sec_callback (void* param)
{
    itracer_stop_timer(AIS140_ADC_ONE_SEC_TIMER_ID);
    itracer_adc_reset_counter();
    ais140_poll_all_analog_ports();
    icom_check_sos_pin_state();
    itracer_start_timer(AIS140_ADC_ONE_SEC_TIMER_ID, AIS140_DEFAULT_DIGITAL_PORT_POLLING_FREQ, 1);
    // itracer_dbg_log(ICOM_LOG_DBG, "ADC 1 Sec \r\n");
}

static int itracer_adc_1_sec_timer_create()
{

    itracer_create_timer( AIS140_ADC_ONE_SEC_TIMER_ID,
                          itracer_adc_1sec_callback, NULL);

    itracer_start_timer( AIS140_ADC_ONE_SEC_TIMER_ID,
                         AIS140_DEFAULT_DIGITAL_PORT_POLLING_FREQ, 1);

    return (0);
}