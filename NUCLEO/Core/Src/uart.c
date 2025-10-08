/**
  * @brief RX Complete Callback — called when 1 byte received
  */
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#include "stm32g4xx_hal_uart.h"
#include<string.h>
#include<stdarg.h>

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef hlpuart1;
#define RX_BUFFER_SIZE 512

#define SERIAL_RX_BUFFER_LEN 512
#define SERIAL_TX_BUFFER_LEN 512
// RX buffer
uint8_t rx_data;
uint8_t rx_buffer[100];
uint16_t rx_index = 0;
uint8_t serial_rx_buffer[SERIAL_RX_BUFFER_LEN];
uint8_t serial_tx_buffer[SERIAL_TX_BUFFER_LEN];

volatile uint16_t serial_rx_index = 0;
volatile uint8_t serial_rx_byte;
char recv_buff3[SERIAL_RX_BUFFER_LEN];
char serial_buffer[512];
char cmd_buffer[512];


// Define a temporary buffer size large enough for your longest debug messages
#define UART_TX_BUFFER_SIZE 256
static char uart_tx_buffer[UART_TX_BUFFER_SIZE];

/**
 * @brief Sends a formatted string over UART1 using vsnprintf.
 * * This function safely handles variable arguments (like printf) and prevents
 * buffer overflow while transmitting the message.
 * * @param format: The format string (e.g., "Value is: %d\r\n")
 * @param ...: The variable arguments.
 */
void UART_Printf(const char *format, ...)
{
    va_list args;
    int len;

    // 1. Initialize the va_list to process the variable arguments
    va_start(args, format);

    // 2. Format the string into the static buffer
    // vsnprintf is essential as it protects against buffer overflow (safe printing)
    len = vsnprintf(uart_tx_buffer, UART_TX_BUFFER_SIZE, format, args);

    // 3. Clean up the va_list
    va_end(args);

    // 4. Transmit the formatted string over UART
    if (len > 0)
    {
        // Adjust length if vsnprintf truncated the string (len >= UART_TX_BUFFER_SIZE)
        if (len >= UART_TX_BUFFER_SIZE)
        {
            len = UART_TX_BUFFER_SIZE - 1; 
        }
        
        // This is the actual transmission using the blocking HAL function.
        // huart1 is assumed to be your global UART handle.
        HAL_UART_Transmit(&huart1, (uint8_t*)uart_tx_buffer, len, HAL_MAX_DELAY);
    }
}



int ais140_parse_commands(char *input_buffer, int read_bytes, int context)
{
    // Pointers for robust trimming and parsing
    char *cmd_start = input_buffer;
    int ret = -1; // Default return is failure

    UART_Printf("DEBUG: Parsing input buffer (len=%d): [%s]\r\n", read_bytes, input_buffer);

    // 1. Skip leading whitespace
    while (*cmd_start == ' ' || *cmd_start == '\t') 
    {
        cmd_start++;
    }

    // 2. Trim trailing whitespace
    size_t len = strlen(cmd_start);
    if (len > 0)
    {
        char *end = cmd_start + len - 1;
        while (end >= cmd_start && (*end == ' ' || *end == '\t'))
        {
            *end = '\0';
            end--;
        }
    }

    // 3. Convert to UPPERCASE for case-insensitivity (CRITICAL for robust parsing)
    char *c = cmd_start;
    while (*c) {
        *c = toupper((unsigned char)*c);
        c++;
    }
    
    UART_Printf("DEBUG: Trimmed & Uppercased command: [%s]\r\n", cmd_start);
    
    // --- Example local commands (Use strcmp on the trimmed, uppercased string) ---
    if (strcmp(cmd_start, "LED ON") == 0)
    {
        // HAL_GPIO_WritePin(GPIOC, OBD_LED_Pin, GPIO_PIN_SET);
        UART_Print("LED turned ON\r\n");
        ret = 0; // Success
    }
    else if (strcmp(cmd_start, "LED OFF") == 0)
    {
        // HAL_GPIO_WritePin(GPIOC, OBD_LED_Pin, GPIO_PIN_RESET);
        UART_Print("LED turned OFF\r\n");
        ret = 0; // Success
    }
    // --- AT_CMD handling ---
    else if (strstr(cmd_start, "AT_CMD:") != NULL)
    {
        // AT_CMD handling... (omitted for brevity, use your existing logic)
        // Ensure you use command_buffer for transmission, not cmd_start
        ret = 0; // Assume success if detected
    }
    else
    {
        // Unknown command
        UART_Printf("Unknown command: %s\r\n", cmd_start);
        ret = -1; // Failure
    }

    return ret;
}
void ais140_process_received_data(char *recv_data, int len)
{
    // The received data should already be null-terminated by the callback,
    // and 'len' is the size *including* the '\n' or '\r'.
    
    memset(cmd_buffer, 0x00, sizeof(cmd_buffer)); 

    if (len > 0)
    {
        // Calculate length of the command *excluding* the final '\n' or '\r'
        size_t command_length = (len > 0) ? len - 1 : 0;
        
        if (command_length > sizeof(cmd_buffer) - 1)
        {
            command_length = sizeof(cmd_buffer) - 1;
        }

        // Copy and terminate the command without the newline/carriage return
        memcpy(cmd_buffer, recv_data, command_length);
        cmd_buffer[command_length] = '\0';
    }
    else
    {
        // Should be caught by the callback, but included for safety.
        UART_Print("DEBUG: Received zero-length data.\r\n");
        return;
    }

    UART_Print("DEBUG: Processing command: [%s] (len=%zu)\r\n", cmd_buffer, strlen(cmd_buffer));

    // Call parse function directly on the stripped command.
    if (ais140_parse_commands(cmd_buffer, strlen(cmd_buffer), 1) == 0)
    {
        UART_Print("DEBUG: Command successfully processed.\r\n");
    }
    else
    {
        UART_Print("DEBUG: Command processing failed.\r\n");
    }

    // ECHO BACK the original data (which includes the newline for a clean terminal)
    UART_Print("ECHO: %s\r\n", recv_data);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        UART_Print("enter to callback\r\n"); // Use \r\n for clear logging
        
        if (serial_rx_index < SERIAL_RX_BUFFER_LEN - 1)
        {
            serial_rx_buffer[serial_rx_index++] = serial_rx_byte;
            UART_Print("enter to callback 1\r\n");
            
            // Check for end of command
            if (serial_rx_byte == '\n' || serial_rx_byte == '\r')
            {
                // FIX: Only process if the buffer has data (data + terminator = index > 1)
                if (serial_rx_index > 1) 
                {
                    UART_Print("enter to callback 2\r\n");
                    serial_rx_buffer[serial_rx_index] = '\0';
                    
                    // Call processing function with the raw buffer
                    ais140_process_received_data((char *)serial_rx_buffer, serial_rx_index);
                } 
                else 
                {
                    UART_Print("DEBUG: Ignored empty command.\r\n");
                }
                
                serial_rx_index = 0; // always reset for next command
            }
        }
        else
        {
            // Buffer overflow — reset safely
            UART_Print("DEBUG: Buffer overflow reset.\r\n");
            serial_rx_index = 0;
        }
        
        // CRITICAL: Restart the interrupt to fill the same single-byte variable
        HAL_UART_Receive_IT(&huart1, &serial_rx_byte, 1); 
    }
}



void MX_USART1_UART_Init(void)
{

  uint8_t start_msg[] = "\r\n=== STM32 USART1 Loopback Test Started ===\r\n";

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
        return -1;
    }

    // Optional: FIFO configuration
    if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
        return -1;
    }

    if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
        return -1;
    }

    if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
    {
        Error_Handler();
        return -1;
    }

    // Start receiving one byte via interrupt
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);

    // Print startup message
    HAL_UART_Transmit(&huart1, start_msg, strlen((char *)start_msg), HAL_MAX_DELAY);

    return 0;

}
/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}
