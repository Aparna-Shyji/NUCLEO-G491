/*gps_parser.c*/
// Parsing globals
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#include "stm32g4xx_hal_uart.h"
#include<string.h>
#include<stdarg.h>
#define NMEA_PARSER_BUFFER_SIZE 200 // Max size of a single NMEA sentence
char nmea_parser_buffer[NMEA_PARSER_BUFFER_SIZE];
char gps_right_line_data[NMEA_PARSER_BUFFER_SIZE];
volatile int front_idx;
int gps_rx_count;
extern uint8_t gps_rx_byte;
extern int gps_rx_index;

void cut_gps_data_start_to_end(char nmea_char)
{
    // Reset/clear buffer if overflowed or a new sentence starts unexpectedly
    // NOTE: nmea_parser_buffer is for the current, single NMEA sentence.
    if ((gps_rx_count >= NMEA_PARSER_BUFFER_SIZE - 1) || (nmea_char == '$'))
    {
    	UART_Print("enter to gps callback 2\r\n");
        gps_rx_count = 0;
        memset(nmea_parser_buffer, 0x00, sizeof(nmea_parser_buffer));
    }

    // Add the current character
    nmea_parser_buffer[gps_rx_count] = nmea_char;

    // Track the '\r' position
    if (nmea_char == '\r')
    {
        front_idx = gps_rx_count;
        UART_Print("enter to gps callback 3\r\n");
    }

    gps_rx_count++;

    // Check for full NMEA line ending with \r\n
    // This is the trigger to perform the slow, final parsing.
    if ((gps_rx_count > 6) && (nmea_parser_buffer[0] == '$') &&
        (nmea_parser_buffer[front_idx] == '\r') && (nmea_char == '\n'))
    {
        // CRITICAL: The operations inside this block are SLOW and blocking the ISR!
        UART_Print("enter to gps callback 4\r\n");
        memset(gps_right_line_data, 0x00, sizeof(gps_right_line_data));
        
        // Copy the complete NMEA sentence
        strncpy(gps_right_line_data, nmea_parser_buffer, gps_rx_count);
        
        // Call the final data processing function
        // This function MUST NOT contain any blocking calls (e.g., HAL_MAX_DELAY)
        // ais140_parse_nmea_data(gps_right_line_data, gps_rx_count);
        
        gps_rx_count = 0; // Reset for next sentence
    }
}