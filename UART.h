#ifndef UART_HEADER
#define	UART_HEADER

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>


void UART1_Initialize(long baudRate,int autoBaud);
uint8_t UART1_Read(void);

#endif	