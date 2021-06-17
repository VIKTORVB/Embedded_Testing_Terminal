#include "UART.h"

void UART1_Initialize(long baudRate, int autoBaud)
{
    // BRGS high speed; MODE Asynchronous 8-bit mode; RXEN enabled; TXEN disabled; ABDEN disabled; 
    U1CON0 = 0x90;

    // RXBIMD Set RXBKIF on rising RX input; BRKOVR disabled; WUE disabled; SENDB disabled; ON enabled; 
    U1CON1 = 0x80;

    if (autoBaud == 0) {
        U1BRGL = baudRate; // move the calculated baud rate to the high and low registers
        U1BRGH = baudRate >> 8;
        // BRGS high speed; MODE Asynchronous 8-bit mode; RXEN enabled; TXEN disabled; ABDEN disabled; 
        U1CON0 = 0x90;
    } else if (autoBaud == 1) {
        // BRGS high speed; MODE Asynchronous 8-bit mode; RXEN enabled; TXEN disabled; ABDEN enabled; 
        U1CON0 = 0xD0;
    }

}
uint8_t UART1_Read(void)
{
    while(!PIR3bits.U1RXIF)
    {
    }
    return U1RXB;
}
