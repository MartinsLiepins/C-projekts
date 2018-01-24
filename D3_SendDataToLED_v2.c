#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "D3_main.h"
//*****************************************************************************
// *********************   EXTERNAL FUNCTION DECLARATION  *********************
//*****************************************************************************




//*****************************************************************************
// *********************   INTERNAL FUNCTION DECLARATION   ********************
//*****************************************************************************


//******************************************************************************
//******************************************************************************
//******************************************************************************


void SendDataToLED(uint32_t sysClock, uint8_t *pointerToColorMap){
  
  uint32_t i;
 
  
  UARTConfigSetExpClk(UART1_BASE, sysClock, UART_BAUD_RATE,
                      UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_TWO |
                      UART_CONFIG_PAR_NONE);
  UARTConfigSetExpClk(UART4_BASE, sysClock, UART_BAUD_RATE,
                      UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_TWO |
                      UART_CONFIG_PAR_NONE);
 
 
  for(i = 0; i < LED_COUNT * 3 * 6; i++){
    UARTCharPut(UART1_BASE, *(pointerToColorMap + i));
    UARTCharPut(UART4_BASE, *(pointerToColorMap + i + LED_COUNT * 3 * 6));
  }
  
  UARTConfigSetExpClk(UART1_BASE, sysClock, UART_BAUD_RATE,
                      UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_TWO |
                      UART_CONFIG_PAR_ZERO);
  UARTConfigSetExpClk(UART4_BASE, sysClock, UART_BAUD_RATE,
                      UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_TWO |
                      UART_CONFIG_PAR_ZERO);  
  UARTCharPut(UART1_BASE, 0x00);
  UARTCharPut(UART4_BASE, 0x00);
}


//*****************************************************************************
// ***********************   INTERNAL FUNCTION DEFINITION   *******************
//*****************************************************************************


