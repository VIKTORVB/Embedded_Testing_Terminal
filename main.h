
#ifndef MAIN_HEADER
#define	MAIN_HEADER

#include <xc.h> // include processor files 
#include "UART.h"
#include "Display.h"
#include "mcc_generated_files/mcc.h"

#define BD PORTBbits.RB3   //Button Down
#define BU PORTBbits.RB2   // Button UP
#define BE PORTBbits.RB1   // Button Enter
#define BB PORTBbits.RB0   // Button Back

int menu_page = 1; // keeps track of the current menu page
int menu_select = 1; // keeps track of the current 
int commStart = 0; // communication start/stop
int SDlog = 0; // non continuous transmission logging start/stop
int ContinuousTransmission = 0; // continuous transmission on/off 
long length; // length of printed message
int errorCheck = 0; // error check on/off
long errorMax = 0; // maximum value
long errorMin = 0; // minimum value
long baudRate = 1667; // baud rate value
long baudRateSelect = 0; // baud rate value chosen by the user
int autoBaud = 0; // auto baud rate detection on/off
char savedMessage[12]; // array for saved message
char messageStr[12]; // array for displayed message
char numberDisplayed[7]; // array for displayed number

const uint8_t Command_map[10] ={
    0x0, // choose page
    0x1, // choose line
    0x2, // choose cursor
    0x3, // specify length of message
    0x4, // clear screen
    0x5, // clear line
    0x6, // start text 
    0x7, // Start communication
    0x8, // SD card logging start
    0x9 // SD card logging stop
};



void Menu(void);
void Error_Message(void);
void Settings_Menu(void);
void Error_Check(void);
void Logging_Check(void);
void UART_Message(void);
void UART_Line(void);
void UART_Cursor(void);
void UART_Page(void);
void UART_Message_Lenght(void);
void UART_Clear_Line(void);
void UART_Command_Mapper(char message);
void UART_Check_Command(void);
void UART_Print(void);
void UART_Check_Comm_Start(void);
void UART_Start(void);
void UART_Settings(void);
void Baud_Rate_Settings(void);
int Toggle_Select(int number, uint8_t line);
void Number_Update(long number);
long Number_Select(long number, long max);
void Baud_rate_Change(void);
void Maximum_Error_Select(void);
void Minimum_Error_Select(void);
void Error_select_range(void);
void Error_Settings(void);
void Continous_Transmission_Length(void);
void Transmission_Settings(void);
int Auto_Boud_Setup(void);
void Auto_Boud(void);
void Menu_Logic(void);
void Cursor_drawing(void);
void Cursor_delete(void);
void Cursor_Position(void);

#endif	

