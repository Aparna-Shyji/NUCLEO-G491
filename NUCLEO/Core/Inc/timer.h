/*timer.h*/
#include "FreeRTOS.h"
#include "timers.h"
#include <string.h>
#include <stdio.h>
typedef void * ql_timer_t;
#define AIS140_DATA_UPLOAD_TIMERID                            0
#define AIS140_ONE_SEC_TIMER                                  1
#define AIS140_TCP_TIMER_ID                                   2
#define AIS140_TIMEOUT_90S_TIMER_ID                           3
#define MAIN_TASK_WD_TIMER_ID                                 4
#define AIS140_TCP_EMERGENCY_TIMER_ID                         5
#define AIS140_TIMEOUT_90S_EMERGENCY_SERVER_TIMER_ID          6
#define AIS140_CLOUD_TASK_WD_TIMER_ID                         7
#define AIS140_HALT_TIMER                                     8
#define AIS140_TIMER_500_MSEC                                 9
#define AIS140_SLEEP_TIMER                                    10
#define AIS140_PROFILE_SWITCH_TIMERID                         11
#define AIS140_MQTT_CONNECT_TIMEOUT_TIMER                     12
#define ITRACER_LICENSE_TIMER                                 13
#define AIS140_PLAT_TASK_WD_TIMER_ID                          14
#define AIS140_MAIN_TASK_ONESEC_TIMER_ID                      15
#define AIS140_CLOUD_FACTORY_RESET_TIMER_ID                   16
#define AIS140_SOCKET_ONE_SEC_TIMER_ID                   	  17
#define AIS140_ADC_ONE_SEC_TIMER_ID                   	      18
#define AIS140_GPRS_TIMER_ID        		           	      19
#define AIS140_DEFAULT_DIGITAL_PORT_POLLING_FREQ            999
#define AIS140_FOTA_TIMER_TIMEOUT                           (1000 * 3)
#define AIS140_DEVICE_FACTORY_RESET_TIMER                   (1000 * 30)      // Time in milliseconds
#define AIS140_GPRS_TIMER_TIMEOUT_FREQ            			(999 * 5)

#define MAX_TIMER_COUNT                                       32


typedef void (*ITRACER_TIMER_CALLBACK)(void *param);

typedef struct {
    TimerHandle_t timer_handle;
    void *p_context;
    ITRACER_TIMER_CALLBACK p_callback;
} TIMER_HANDLE;

extern TIMER_HANDLE timer_array[MAX_TIMER_COUNT];

int itracer_create_timer(int timer_id, ITRACER_TIMER_CALLBACK p_callback, void *p_context);
int itracer_start_timer(int timer_id, int m_sec, int mode);
int itracer_stop_timer(int timer_id);
void itracer_timer_init(void);
extern void itracer_test_timer_init(void);
extern int itracer_start_halt_timer(void);
extern int itracer_start_sleep_timer(void);
extern int itracer_stop_sleep_timer(void);
extern int itracer_stop_halt_timer(void);