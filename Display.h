
#ifndef DISPLAY_HEADER
#define	DISPLAY_HEADER

#include <xc.h> // include processor files  
#include "mcc_generated_files/mcc.h" // include code generator files for pin configurations

#define GlcdDataBus      PORTD // Display data bus

#define RS PORTAbits.RA3   // Display 
#define RW PORTAbits.RA4   // Display Read/Write bit
#define EN PORTAbits.RA0   // Display Enable bit
#define CS1 PORTAbits.RA1  // Display page select
#define CS2 PORTAbits.RA2  // Display page select

void GLCD_Initialise(void);
void Select_Page(int);
void Command_Write(char);
void Data_Write(char);
void Display_Character(char);
void Display_Message(char message[]);
void Clear_Display(void);
#endif	

