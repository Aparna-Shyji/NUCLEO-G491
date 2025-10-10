/*
 * gpio.c
 *
 *  Created on: Oct 4, 2025
 *      Author: Architha
 */

#include"main.h"
#include"type.h"

static ais140_uchar    vts_car_battery_port_state = 0;
static ais140_uchar    vts_battery_port_state =0;
void ais140_gps_led_state(uint8_t status)
{
    if (status == 1) {
        HAL_GPIO_WritePin(GPIOA, GPS_LED_Pin, GPIO_PIN_SET);
    } else if (status == 0) {
        HAL_GPIO_WritePin(GPIOA, GPS_LED_Pin, GPIO_PIN_RESET);
    }
}

void ais140_gprs_led_state(uint8_t status)
{
    if (status == 1) {
        HAL_GPIO_WritePin(GPIOA, GPRS_LED_Pin, GPIO_PIN_SET);
    } else if (status == 0) {
        HAL_GPIO_WritePin(GPIOA, GPRS_LED_Pin, GPIO_PIN_RESET);
    }
}

void ais140_obd_led_state(uint8_t status)
{
    if (status == 1) {
        HAL_GPIO_WritePin(GPIOC, OBD_LED_Pin, GPIO_PIN_SET);
    } else if (status == 0) {
        HAL_GPIO_WritePin(GPIOC, OBD_LED_Pin, GPIO_PIN_RESET);
    }
}

void ais140_gps_en_state(uint8_t status)
{
    if (status == 1) {
        HAL_GPIO_WritePin(GPIOC, GPS_EN_Pin, GPIO_PIN_SET);
    } else if (status == 0) {
        HAL_GPIO_WritePin(GPIOC, GPS_EN_Pin, GPIO_PIN_RESET);
    }
}

void ais140_wifi_htpt_led_state(uint8_t status)
{
    if (status == 1) {
        HAL_GPIO_WritePin(GPIOB, WIFI_HTPT_LED_Pin, GPIO_PIN_SET);
    } else if (status == 0) {
        HAL_GPIO_WritePin(GPIOB, WIFI_HTPT_LED_Pin, GPIO_PIN_RESET);
    }
}

void ais140_bat_en_state(uint8_t status)
{
    if (status == 1) {
        HAL_GPIO_WritePin(GPIOB, BAT_EN_Pin, GPIO_PIN_SET);
    } else if (status == 0) {
        HAL_GPIO_WritePin(GPIOB, BAT_EN_Pin, GPIO_PIN_RESET);
    }
}

void itracer_set_car_battery_port_physical_state(int port_state)
{
    vts_car_battery_port_state = port_state;
}

ais140_uchar itracer_get_car_battery_port_physical_state(void)
{
    return (vts_car_battery_port_state);
}

void itracer_set_battery_port_physical_state(int port_state)
{
    vts_battery_port_state = port_state;
}

ais140_uchar itracer_get_battery_port_physical_state(void)
{
    return (vts_battery_port_state);
}
