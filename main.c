#include "main.h"

FATFS drive;
FIL file;

void Save_Data() {
    UINT actualLength;
    char newLine[] = "\r\n"; // declare an array that would act as a new line when saved to the SD card
    if (SD_SPI_IsMediaPresent() == false) { // check if there is anything connected to the SPI ports
        return;
    }
    if (f_mount(&drive, "0:", 1) == FR_OK) { // mount the SD card
        if (f_open(&file, "LOG.TXT", FA_OPEN_APPEND | FA_WRITE) == FR_OK) { // open the file and append to it the new message
            f_write(&file, savedMessage, sizeof (savedMessage) - 1, &actualLength); // write message
            f_write(&file, newLine, sizeof (newLine) - 1, &actualLength); // write new line
            f_sync(&file); // sync the file in case of any error during transmission
            memset(savedMessage, 0, sizeof savedMessage); // clear the message array

        }
    }
}

void Menu() // print the main menu
{
    Clear_Display();
    while (1) {
        Select_Page(1);
        Command_Write(0xb8);
        Command_Write(0b01000000);
        Display_Message("Menu");

        // display on page 0 line 0
        Select_Page(0);
        Command_Write(0b10111000);
        Command_Write(0b01101000);
        Display_Message("Main");

        //Display on Page 0, Line 3
        Select_Page(0);
        Command_Write(0b10111010);
        Command_Write(0b01000000);
        Display_Message("Start");

        // Display on page 0, Line 5
        Command_Write(0b10111100);
        Command_Write(0b01000000);
        Display_Message("Settings");

        Cursor_Position(); // call cursor position
    }


}

void Error_Message() // Print error message
{
    Clear_Display();
    Select_Page(0);
    while (BB == 0) {
        Command_Write(0b10111000);
        Command_Write(0b01000000);
        Display_Message("Error");
        Command_Write(0b10111001);
        Command_Write(0b01000000);
        Display_Message("Please");
        Command_Write(0b10111010);
        Command_Write(0b01000000);
        Display_Message("Start");
        Command_Write(0b10111011);
        Command_Write(0b01000000);
        Display_Message("Transmission");
        Command_Write(0b10111100);
        Command_Write(0b01000000);
        Display_Message("With a");
        Command_Write(0b10111101);
        Command_Write(0b01000000);
        Display_Message("Command");
    }

}

void Settings_Menu() // Print the setting menu
{
    Clear_Display();
    Select_Page(0);
    Command_Write(0b10111000);
    Command_Write(0b01011100);
    Display_Message("System");

    Select_Page(1);
    Command_Write(0b10111000);
    Command_Write(0b01000000);
    Display_Message("Settings");
    //Display on Page 0, Line 3
    Select_Page(0);
    Command_Write(0b10111010);
    Command_Write(0b01000000);
    Display_Message("UART");

    // Display on page 0, Line 5
    Command_Write(0b10111100);
    Command_Write(0b01000000);
    Display_Message("Error");

    Cursor_drawing(); // call cursor position
}

void Error_Check() {
    if (errorCheck == 1) { // if error checking is active
        int messageInt = atoi(messageStr); // push the string to integer
        if ((messageInt > errorMax || messageInt < errorMin) && messageInt > 0) { // if the integer is out of range
            Display_Character('!'); // display warning after the number
            messageStr[length + 1] = '!'; // attach warning to the array so it can be saved to the SD card if active
        } else { // if the integer is in range print a blank after it , to delete any warning messages if the integers are being printed in the same place on the screen
            Display_Character(' ');
        }
    }
}

void Logging_Check() {
    if (SDlog == 1) { // if SD card logging is active
        strcpy(savedMessage, messageStr); // copy the string
        Save_Data(); // call the SD card logging function
        memset(messageStr, 0, sizeof messageStr); // clear the string
    }
}

void UART_Message() {
    char message;

    for (int i = 0; i < length; i++) { // loop to read the full message depending on user specified length
        message = UART1_Read();
        messageStr[i] = message; // save the message in an array
        Display_Character(message); // print message char by char
    }
    Error_Check(); // call error checking function
    Logging_Check(); // call logging function
}

void UART_Line() // read the specified line and send it to the display
{
    uint8_t line;
    line = UART1_Read();
    Command_Write(0xB8 + line);
}

void UART_Cursor() // read the specified cursor position and send it to the display
{
    uint8_t position;
    position = UART1_Read();
    Command_Write(0x40 + position);
}

void UART_Page() // read the specified page and send it to the display
{
    int page;
    page = UART1_Read();
    Select_Page(page);
}

void UART_Message_Lenght() // read the specified length of the message
{
    length = UART1_Read();
}

void UART_Clear_Line() {
    length = 12; // length of the whole line
    Command_Write(0x40); // set the cursor to 0
    Display_Message("            "); // print blank spaces to clear the line
}

void UART_Command_Mapper(char message) // route the program to the specific function depending on the command received
{
    switch (message) {
        case 0x0: UART_Page();
            break;
        case 0x1: UART_Line();
            break;
        case 0x2: UART_Cursor();
            break;
        case 0x3: UART_Message_Lenght();
            break;
        case 0x4: Clear_Display();
            break;
        case 0x5: UART_Clear_Line();
            break;
        case 0x6: UART_Message();
            break;
        case 0x7:
            break;
        case 0x8: SDlog = 1;
            break;
        case 0x9: SDlog = 0;
            break;
    }
}

void UART_Check_Command() {
    char message;
    message = UART1_Read(); // read from UART
    for (int i = 0; i < 8; i++) { // loop trough the array of commands
        if (message == Command_map[i]) { // if the read message matches to any command send the message to 
            UART_Command_Mapper(message); // the routing function
        } else if (i == 9) { // if it dose not match with any command print error message
            //Error_Message();
        }
    }
}

void UART_Print() {// UART continuous transmission
    while (BB == 0) {
        uint8_t line;
        char data;
        Clear_Display();
        for (line = 0; line < 8; line++) // loop through all the 8lines 
        {
            Select_Page(0); // Select page0 and set the cursor to first position 
            Command_Write(0x40);
            Command_Write(0xB8 + line); // set the line to 0 + the iteration of the loop
            UART_Message(); // print received message
            if (line == 7) // when all lines have been written to move to next page
            {
                for (line = 0; line < 8; line++) { // repeat the same process for the next page
                    Select_Page(1);
                    Command_Write(0x40);
                    Command_Write(0xB8 + line);
                    UART_Message();
                    if (BB == 1) {// if the back button is pressed break out of loop
                        break;
                    }
                }
            }
            else if (BB == 1) {// if the back button is pressed break out of loop
                break;
            }
        }
    }
}

void UART_Check_Comm_Start() { // check if the first read message via the UART is the start transmission command
    char message;
    message = UART1_Read();
    if (message == 0x7) {
        commStart = 1; // if it is set the communication start flag high
    } else {
        commStart = 0; // if not set the flag low and print error
       // Error_Message();
    }
}

void UART_Start() {
    UART1_Initialize(baudRate, autoBaud); // initialise the UART and pass to it the baud rate and the automatic baud flag bit
    Clear_Display();

    while (BB == 0) {
        if (ContinuousTransmission == 0) {// if continuous TR is off and Communication start is high
            if (commStart == 1) {
                UART_Check_Command(); // start UART communication by checking for the first command
            } else if (commStart == 0) { // if the communication start is low check for it and try again
                UART_Check_Comm_Start();
            }
        } else if (ContinuousTransmission == 1) { // if continuous TR is on start communication
            UART_Print();
        }
    }
}

void UART_Settings()// print the settings menu
{
    Clear_Display();
    Select_Page(0);
    Command_Write(0b10111000);
    Command_Write(0b01101000);
    Display_Message("UART");

    Select_Page(1);
    Command_Write(0b10111000);
    Command_Write(0b01000000);
    Display_Message("Settings");

    //Display on Page 0, Line 3
    Select_Page(0);
    Command_Write(0b10111010);
    Command_Write(0b01000000);
    Display_Message("Transmission");

    // Display on page 0, Line 5
    Command_Write(0b10111100);
    Command_Write(0b01000000);
    Display_Message("Baud rate");

    Cursor_drawing(); // draw cursor
}

void Baud_Rate_Settings() // print baud rate settings
{
    char autoBaudChar[1];
    sprintf(autoBaudChar, "%d", autoBaud); // push the automatic baud flag to char

    Clear_Display();
    Select_Page(0);
    Command_Write(0b10111000);
    Command_Write(0b01011100);
    Display_Message("Baud");

    Select_Page(1);
    Command_Write(0b10111000);
    Command_Write(0b01000000);
    Display_Message("Settings");

    //Display on Page 0, Line 3
    Select_Page(0);
    Command_Write(0b10111010);
    Command_Write(0b01000000);
    Display_Message("Automatic");

    Select_Page(1);
    Command_Write(0b10111010);
    Command_Write(0b01100000);
    Display_Message(autoBaudChar); // print the flag

    Select_Page(0);
    // Display on page 0, Line 5
    Command_Write(0b10111100);
    Command_Write(0b01000000);
    Display_Message("Custom");

    Cursor_drawing();
}

int Toggle_Select(int number, uint8_t line) {
    char Char[1];

    if (number == 0) { // toggle the number received 
        number = 1; // and print it out on the line received
        Select_Page(1);
        Command_Write(0xB8 + line);
        Command_Write(0b01100000);
        sprintf(Char, "%d", number);
        Display_Message(Char);
    } else if (number == 1) {
        number = 0;
        Select_Page(1);
        Command_Write(0xB8 + line);
        Command_Write(0b01100000);
        sprintf(Char, "%d", number);
        Display_Message(Char);
    }
    __delay_ms(200); // delay for a considerable amount of time   
    return number; // to stop the output from racing

}

void Number_Update(long number) {
    sprintf(numberDisplayed, "%d", number); // push the received integer to char array

    Select_Page(1);
    Command_Write(0b10111011);
    Command_Write(0b01000000);
    Display_Message(numberDisplayed); // print the array

    if (number <= 9) { // depending on how large the number is clear the whole line after it
        Command_Write(0b10111011); // this is done so that no unexpected characters are printed 
        Command_Write(0b01000101); // when a number goes down in figures
        Display_Message("         ");
    }
    if (number <= 99) {
        Command_Write(0b10111011);
        Command_Write(0b01001010);
        Display_Message("        ");
    }
    if (number <= 999) {
        Command_Write(0b10111011);
        Command_Write(0b01001111);
        Display_Message("      ");
    }
    if (number <= 9999) {
        Command_Write(0b10111011);
        Command_Write(0b01010100);
        Display_Message("     ");
    }
    if (number <= 99999) {
        Command_Write(0b10111011);
        Command_Write(0b01011001);
        Display_Message("    ");
    }
    if (number <= 999999) {
        Command_Write(0b10111011);
        Command_Write(0b01011110);
        Display_Message("   ");
    }



}

long Number_Select(long number, long max) {
    int longPress = 0;
    if (BU == 1) { // if up is pressed
        while (BU == 1) {// while up is pressed
            longPress++; // increment the long press counter
            if (longPress < 10 && number <= max) { // increment the number by different values depending
                number++; // on how long the button has been pressed for
                __delay_ms(200);
            }
            if (longPress >= 10 && number >= 9 && number <= max) {
                number += 10;
                __delay_ms(50);
            }
            if (longPress >= 30 && number >= 100 && number <= max) {
                number += 100;
            }
            Number_Update(number);
        }
    }
    if (BD == 1) {
        while (BD == 1) {
            longPress++;
            if (longPress < 10 && number > 0) {
                number--;
                __delay_ms(200);
            }
            if (longPress >= 10 && number >= 9 && number > 0) {
                number -= 10;
                __delay_ms(50);
            }
            if (longPress >= 30 && number >= 100 && number > 0) {
                number -= 100;
            }
            Number_Update(number);
        }
    }
    return number;
}

void Baud_rate_Change() { // function to change the baud rate of the device
    Clear_Display();
    sprintf(numberDisplayed, "%d", baudRateSelect); // push the integer to char
    while (BB == 0) {
        Select_Page(0);
        Command_Write(0b10111010);
        Command_Write(0b01000000);
        Display_Message("Baud");

        // Display on page 0, Line 5
        Command_Write(0b10111100);
        Command_Write(0b01000000);
        Display_Message("Rate");

        Select_Page(1);
        Command_Write(0b10111011);
        Command_Write(0b01000000);
        Display_Message(numberDisplayed); // print the baud rate

        baudRateSelect = Number_Select(baudRateSelect, 999999); // call the number select function by passing to it the 
    } // current baud rate and the maximum value for the baud rate
    baudRate = (64000000 / baudRateSelect / 4) - 1; // perform the baud rate calculation (derived from the data sheet) 
    Clear_Display();
}

void Maximum_Error_Select() {  // function for selecting the maximum range of the error checking
    Clear_Display();
    sprintf(numberDisplayed, "%d", errorMax);
    while (BB == 0) {
        Select_Page(0);
        Command_Write(0b10111010);
        Command_Write(0b01000000);
        Display_Message("Maximum");

        // Display on page 0, Line 5
        Command_Write(0b10111100);
        Command_Write(0b01000000);
        Display_Message("Range");

        Select_Page(1);
        Command_Write(0b10111011);
        Command_Write(0b01000000);
        Display_Message(numberDisplayed);

        errorMax = Number_Select(errorMax, 999999);
    }
    Clear_Display();
}

void Minimum_Error_Select() { // function for selecting the minimum range of the error checking
    Clear_Display();
    sprintf(numberDisplayed, "%d", errorMin);
    while (BB == 0) {
        Select_Page(0);
        Command_Write(0b10111010);
        Command_Write(0b01000000);
        Display_Message("Minimum");

        // Display on page 0, Line 5
        Command_Write(0b10111100);
        Command_Write(0b01000000);
        Display_Message("Range");

        Select_Page(1);
        Command_Write(0b10111011);
        Command_Write(0b01000000);
        Display_Message(numberDisplayed);

        errorMin = Number_Select(errorMin, 999999);
    }
    Clear_Display();
}

void Error_select_range() { // print error select range menu
    Clear_Display();
    Select_Page(0);
    Command_Write(0b10111000);
    Command_Write(0b01011100);
    Display_Message("Error");

    Select_Page(1);
    Command_Write(0b10111000);
    Command_Write(0b01000000);
    Display_Message("Settings");

    Select_Page(0);
    Command_Write(0b10111010);
    Command_Write(0b01000000);
    Display_Message("Maximum");

    // Display on page 0, Line 5
    Command_Write(0b10111100);
    Command_Write(0b01000000);
    Display_Message("Minimum");

    Cursor_drawing();
}

void Error_Settings() { // print the error Settings menu
    char errorcheckChar[1];
    sprintf(errorcheckChar, "%d", errorCheck);

    Clear_Display();
    Select_Page(0);
    Command_Write(0b10111000);
    Command_Write(0b01011100);
    Display_Message("Error");

    Select_Page(1);
    Command_Write(0b10111000);
    Command_Write(0b01000000);
    Display_Message("Settings");

    //Display on Page 0, Line 3
    Select_Page(0);
    Command_Write(0b10111010);
    Command_Write(0b01000000);
    Display_Message("Error Check");

    Select_Page(1);
    Command_Write(0b10111010);
    Command_Write(0b01100000);
    Display_Message(errorcheckChar);

    Select_Page(0);
    // Display on page 0, Line 5
    Command_Write(0b10111100);
    Command_Write(0b01000000);
    Display_Message("Decimal Range");

    Cursor_drawing();
}

void Continous_Transmission_Length() {// function for changing the length of the message in continuous TR 
    Clear_Display();
    sprintf(numberDisplayed, "%d", length);
    while (BB == 0) {
        Select_Page(0);
        Command_Write(0b10111010);
        Command_Write(0b01000000);
        Display_Message("Transmission");

        // Display on page 0, Line 5
        Command_Write(0b10111100);
        Command_Write(0b01000000);
        Display_Message("Length");

        Select_Page(1);
        Command_Write(0b10111011);
        Command_Write(0b01000000);
        Display_Message(numberDisplayed);

        length = Number_Select(length, 11);
    }
    Clear_Display();
}

void Transmission_Settings() { // print the transmission setting s menu
    char ContinuousChar[1];
    char SDloggingChar[1];
    sprintf(ContinuousChar, "%d", ContinuousTransmission);
    sprintf(SDloggingChar, "%d", SDlog);

    Clear_Display();
    Select_Page(0);
    Command_Write(0b10111000);
    Command_Write(0b01000000);
    Display_Message("Transmission");

    Select_Page(1);
    Command_Write(0b10111000);
    Command_Write(0b01000000);
    Display_Message("Settings");

    //Display on Page 0, Line 3
    Select_Page(0);
    Command_Write(0b10111010);
    Command_Write(0b01000000);
    Display_Message("Continuous");

    Select_Page(1);
    Command_Write(0b10111010);
    Command_Write(0b01100000);
    Display_Message(ContinuousChar);
    Command_Write(0b01100101);
    Display_Character(' ');

    Select_Page(0);
    // Display on page 0, Line 5
    Command_Write(0b10111100);
    Command_Write(0b01000000);
    Display_Message("SD Logging");

    Select_Page(1);
    Command_Write(0b10111100);
    Command_Write(0b01100000);
    Display_Message(SDloggingChar);

    // Display on page 0, Line 7
    Select_Page(0);
    Command_Write(0b10111110);
    Command_Write(0b01000000);
    Display_Message("Length");

    Cursor_drawing();

}

int Auto_Boud_Setup() {
    char syncChar;
    commStart = 1; // manually set the communication start flag
    UART1_Initialize(baudRate, autoBaud); // initialise the UART by passing the current Baud which should not matter and pass the automatic baud rate flag 
    for (int i = 0; i < 3; i++) {         // loop trough three times just to be sure that the operation is complete
        syncChar = UART1_Read();          // read from the UART, the first char will be corrupted, so reading will clear the register
        if (syncChar == 'U') {            // on the second and third read see if a character has been received
            return 1;                     // if a character has been received, then the setup is complete and successful
        } else if (syncChar != 'U' && i == 3) { // if by the third read no character is received 
            return 0;                           // then the operation has not been completed successfully
        }
    }
}

void Auto_Boud() { // function to set up the baud rate automatically
    Clear_Display();
    Select_Page(0);
    Command_Write(0b10111000);
    Command_Write(0b01000000);
    Display_Message("Please send");
    Command_Write(0b10111001);
    Command_Write(0b01000000);
    Display_Message("Three");
    Command_Write(0b10111010);
    Command_Write(0b01000000);
    Display_Message("Consecutive");
    Command_Write(0b10111011);
    Command_Write(0b01000000);
    Display_Message("U characters");

    if (Auto_Boud_Setup() == 1) {  // if the function returns a 1 then the operation has been successful
        Command_Write(0b10111101);
        Command_Write(0b01000000);
        Display_Message("Complete");
        autoBaud = 1;
    } else { // if not then the operation has not been successful
        Command_Write(0b10111101);
        Command_Write(0b01000000);
        Display_Message("Error");
        autoBaud = 0;
    }
    while (BB == 0); // wait for back button to be pressed

}

void Menu_Logic() {  // Main menu logic
    {
        // Main Page
        if (BE == 1 && menu_page == 1 && menu_select == 1) {
            UART_Start();
        }
        if (BE == 1 && menu_page == 1 && menu_select == 2) {
            menu_page = 2;
            __delay_ms(100);
            Settings_Menu();
        }

        // Settings page

        if (menu_page == 2 && menu_select == 1 && BE == 1) {
            menu_page = 3;
            __delay_ms(100);
            UART_Settings();
        }
        if (menu_page == 2 && menu_select == 2 && BE == 1) {
            menu_page = 4;
            __delay_ms(100);
            Error_Settings();
        }

        // UART settings

        if (menu_page == 3 && menu_select == 1 && BE == 1) {
            menu_page = 7;
            __delay_ms(100);
            Transmission_Settings();
        }
        if (menu_page == 3 && menu_select == 2 && BE == 1) {
            menu_page = 6;
            __delay_ms(100);
            Baud_Rate_Settings();
        }

        // Error settings

        if (menu_page == 4 && menu_select == 1 && BE == 1) {
            __delay_ms(100);
            errorCheck = Toggle_Select(errorCheck, 2);
        }
        if (menu_page == 4 && menu_select == 2 && BE == 1) {
            menu_page = 5;
            __delay_ms(100);
            Error_select_range();
        }

        // Error check range

        if (menu_page == 5 && menu_select == 1 && BE == 1) {
            __delay_ms(100);
            Maximum_Error_Select();
        }
        if (menu_page == 5 && menu_select == 2 && BE == 1) {
            __delay_ms(100);
            Minimum_Error_Select();
        }

        // Baud rate settings

        if (menu_page == 6 && menu_select == 1 && BE == 1) {
            __delay_ms(100);
            autoBaud = Toggle_Select(autoBaud, 2);
            Auto_Boud();
        }
        if (menu_page == 6 && menu_select == 2 && BE == 1) {
            __delay_ms(100);
            Baud_rate_Change();
        }

        // Transmission settings

        if (menu_page == 7 && menu_select == 1 && BE == 1) {
            __delay_ms(100);
            ContinuousTransmission = Toggle_Select(ContinuousTransmission, 2);
        }
        if (menu_page == 7 && menu_select == 2 && BE == 1) {
            __delay_ms(100);
            SDlog = Toggle_Select(SDlog, 4);
        }
        if (menu_page == 7 && menu_select == 3 && BE == 1) {
            __delay_ms(100);
            Continous_Transmission_Length();
        }

    }
}

void Cursor_drawing() {
    Select_Page(1);
    switch (menu_select) { // depending on where the cursor integer is print the cursor
        case 1:
        {
            Command_Write(0b10111010); // Line 3
            Command_Write(0b01111000); // position 56
            Display_Character('<'); //Draw cursor
            break;
        }
        case 2:
        {
            Command_Write(0b10111100); // Line 5
            Command_Write(0b01111000); // position 56
            Display_Character('<'); //Draw cursor
            break;
        }
        case 3:
            Command_Write(0b10111110); // Line 7
            Command_Write(0b01111000); // position 56
            Display_Character('<'); //Draw cursor
            break;
    }
}

void Cursor_delete() { // delete the cursor before it is printed again
    Select_Page(1);
    Command_Write(0b10111010); // Line 3
    Command_Write(0b01111000); // position 56
    Display_Character(' ');
    Command_Write(0b10111011); // Line 4
    Command_Write(0b01111000); // position 56
    Display_Character(' ');
    Command_Write(0b10111100); // Line 5
    Command_Write(0b01111000); // position 56
    Display_Character(' ');
    Command_Write(0b10111101); // Line 6
    Command_Write(0b01111000); // position 56
    Display_Character(' ');
    Command_Write(0b10111110); // Line 7
    Command_Write(0b01111000); // position 56
    Display_Character(' ');
    Command_Write(0b10111111); // Line 8
    Command_Write(0b01111000); // position 56
    Display_Character(' ');
}

void Cursor_Position() { // depending on the page move the cursor respective to the button presses
    Cursor_drawing();
    while (BB == 0) {
        if (BE == 1)
            Menu_Logic();
            __delay_ms(100);
        if (BD == 1 && menu_page <= 6 && menu_select < 2) {
            menu_select++;
            Cursor_delete();
            Cursor_drawing();
            __delay_ms(100);  // add a small delay to make switching between positions
        }                     // a lot smoother
        if (BU == 1 && menu_page <= 6 && menu_select > 1) {
            menu_select--;
            Cursor_delete();
            Cursor_drawing();
            __delay_ms(100);
        }
        if (menu_page == 7 && BD == 1 && menu_select < 3) {
            menu_select++;
            Cursor_delete();
            Cursor_drawing();
            __delay_ms(100);
        }
        if (menu_page == 7 && BU == 1 && menu_select > 1) {
            menu_select--;
            Cursor_delete();
            Cursor_drawing();
            __delay_ms(100);
        }
    }
    Clear_Display();
    menu_page = 1; // when back is pressed reset the page and clear the display
    f_close(&file); // close the file
        f_mount(0, "0:", 0); // dismount the SD card
}

void main() {
    SYSTEM_Initialize();// initialise the system
    GLCD_Initialise();// initialise the display
    Clear_Display(); // clear the display
    Menu(); // start the menu
}