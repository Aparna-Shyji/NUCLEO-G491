/*
 * adc.c
 *
 *  Created on: Oct 4, 2025
 *      Author: HP
 */


#include "stm32g4xx_hal.h"
#include "type.h"

// Assuming these macros are defined somewhere
#define MAIN_PWR_ADC_CHANNEL      ADC_CHANNEL_6   // Main battery
#define INTERNAL_BAT_ADC_CHANNEL  ADC_CHANNEL_7   // Standby battery
#define CAR_BATTERY_ADC_RESOLUTION   0.025f       // Example: scale factor to convert ADC to volts
#define CAR_BATTERY_DIODE_RESOLUTION 0.3f         // Voltage drop of diode
#define STANDBY_BATTERY_ADC_RESOLUTION 0.00144f   // Scale factor for internal battery
#define MAIN_PWR_INDEX                0
#define BAT_PWR_INDEX                 1
#define MAX_ADC_CHANNELS              2
#define CAR_BATTERY_CONNECTION_THRESHOLD                          250 // 297 for 8v, Below 8V it will be detected as Main Pwr Disconnected
#define ADC_REF_VOLTAGE        3.3f      // or your Vref pin voltage
#define ADC_RESOLUTION_MAX     4095.0f   // for 12-bit ADC

extern ADC_HandleTypeDef hadc1;
static ais140_uchar                                               internal_battery_trigger_flag = 0;


extern void itracer_set_car_battery_port_physical_state(int port_state);
extern void itracer_set_battery_port_physical_state(int port_state);
typedef struct {
    int voltage_values;
    uint16_t limit_value;
} ADC_VOLTAGE_DB;

ADC_VOLTAGE_DB adc_voltage_db[8];   // Example: 8 ADC channels

/*----------------------------------------------------------
 * Read ADC Channel Value
 *----------------------------------------------------------*/
int itracer_get_adc_channel_values(int channel, int *adc_value)
{
    uint32_t raw_adc = 0;
    float voltage = 0.0f;
    ADC_ChannelConfTypeDef sConfig = {0};

    /* Configure the selected ADC channel */
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
        return -1;
    }

    /* Start ADC Conversion */
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        raw_adc = HAL_ADC_GetValue(&hadc1);
        voltage = (raw_adc / ADC_RESOLUTION_MAX) * ADC_REF_VOLTAGE;  // In volts

        *adc_value = (int)(voltage * 1000);  // Store in mV (like EC200 style)
        adc_voltage_db[channel].voltage_values = *adc_value;
    }
    HAL_ADC_Stop(&hadc1);

    return 0;
}

/*----------------------------------------------------------
 * Set ADC Channel Voltage (manual override)
 *----------------------------------------------------------*/
int ais140_set_adc_channel_voltage(int channel, int data)
{
    adc_voltage_db[channel].voltage_values = data;
    return 0;
}

/*----------------------------------------------------------
 * Set Voltage Limit for a Channel
 *----------------------------------------------------------*/
void ais140_set_voltage_limit(int channel, uint16_t limit)
{
    adc_voltage_db[channel].limit_value = limit;
}

/*----------------------------------------------------------
 * Trigger Flag Management
 *----------------------------------------------------------*/
void ais140_set_trigger_flag(int channel, uint8_t trigger_flag)
{
    internal_battery_trigger_flag = trigger_flag;
}

uint8_t ais140_get_trigger_flag(int channel)
{
    return internal_battery_trigger_flag;
}

// Read main battery voltage
float ais140_get_main_battery_voltage(void)
{
    float adc_value = 0.0f;
    uint32_t raw_adc = 0;

    // Configure ADC channel for main battery
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = MAIN_PWR_ADC_CHANNEL;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_12CYCLES_5; // or as per your init
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK)
    {
        raw_adc = HAL_ADC_GetValue(&hadc1);
        adc_value = raw_adc * CAR_BATTERY_ADC_RESOLUTION + CAR_BATTERY_DIODE_RESOLUTION;
    }
    HAL_ADC_Stop(&hadc1);

    // Optional: check if battery physically connected
    if (itracer_get_car_battery_port_physical_state() == 0)
    {
        adc_value = 0.0f;
    }

#ifdef DEBUG_LOG
    if (itracer_get_dbg_flag() & ICOM_LOG_INFO)
    {
        char buffer[100];
        itracer_sprintf(buffer, "ADC-VALUE : Main Bat raw=%lu, voltage=%f V\r\n", raw_adc, adc_value);
        itracer_dbg_log(ICOM_LOG_INFO, buffer);
    }
#endif

    return adc_value;
}

// Read standby/internal battery voltage
float ais140_get_standby_battery_voltage(void)
{
    float adc_value = 0.0f;
    uint32_t raw_adc = 0;

    // Configure ADC channel for internal battery
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = INTERNAL_BAT_ADC_CHANNEL;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_12CYCLES_5; // adjust as needed
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK)
    {
        raw_adc = HAL_ADC_GetValue(&hadc1);
        adc_value = raw_adc * STANDBY_BATTERY_ADC_RESOLUTION;
    }
    HAL_ADC_Stop(&hadc1);

#ifdef DEBUG_LOG
    if (itracer_get_dbg_flag() & ICOM_LOG_INFO)
    {
        char buffer[100];
        itracer_sprintf(buffer, "ADC-VALUE : Standby Bat raw=%lu, voltage=%f V\r\n", raw_adc, adc_value);
        itracer_dbg_log(ICOM_LOG_INFO, buffer);
    }
#endif

    return adc_value;
}
/* --------------------------------------------------------------------------
 * Function: ais140_poll_all_analog_ports
 * Description:
 *  - Polls main power & battery power ADC channels
 *  - Updates adc_voltage_db[]
 *  - Sets their corresponding physical port states
 * -------------------------------------------------------------------------- */
void ais140_poll_all_analog_ports(void)
{
    int i = 0;
    int adc_value = 0;

    // Step 1: Poll both ADC channels
    for (i = 0; i < MAX_ADC_CHANNELS; i++) {
        if (i == MAIN_PWR_INDEX)
            itracer_get_adc_channel_values(MAIN_PWR_ADC_CHANNEL, &adc_value);
        else if (i == BAT_PWR_INDEX)
            itracer_get_adc_channel_values(INTERNAL_BAT_ADC_CHANNEL, &adc_value);
    }

    // Step 2: Update physical states based on ADC thresholds
    if (adc_voltage_db[MAIN_PWR_INDEX].voltage_values > CAR_BATTERY_CONNECTION_THRESHOLD) {
        itracer_set_car_battery_port_physical_state(1);
    } else {
        itracer_set_car_battery_port_physical_state(0);
    }

    if (adc_voltage_db[BAT_PWR_INDEX].voltage_values > CAR_BATTERY_CONNECTION_THRESHOLD) {
        itracer_set_battery_port_physical_state(1);
    } else {
        itracer_set_battery_port_physical_state(0);
    }

    // Step 3: Debug print (optional)
#ifdef DEBUG_LOG
    snprintf(buffer_log, sizeof(buffer_log),
             "ADC VALUES: Main=%lu, Battery=%lu\r\n",
             (unsigned long)adc_voltage_db[MAIN_PWR_INDEX].voltage_values,
             (unsigned long)adc_voltage_db[BAT_PWR_INDEX].voltage_values);
    itracer_dbg_log(ICOM_LOG_INFO, buffer_log);
#endif

    // Step 4: Call additional processing if needed
    ais140_adc_port_check_data();
}
