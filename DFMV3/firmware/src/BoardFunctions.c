#include "GlobalIncludes.h"

unsigned char dfmID;
unsigned char isInDarkMode;
unsigned char ledStatusBits;
unsigned char usingNewPortOnly;

void InitializeBoard(){
    // Set All ID Selectors  

    //RX485_DISABLE_SEND();
    BLUELED_OFF();
    Nop();Nop();Nop();
    GREENLED_OFF();
    Nop();Nop();Nop();
    REDLED_OFF();
    Nop();Nop();Nop();
    YELLOWLED_OFF();
    Nop();Nop();Nop();
    //DelayMs(1000);
    dfmID = GETIDSELECTOR_VALUE();      
    usingNewPortOnly = !SWITCH_PORT; 
    if(usingNewPortOnly)
        YELLOWLED_ON();
    else
        YELLOWLED_OFF();
    RX485_DISABLE_SEND();
}

void SetDarkMode(unsigned char mode){    
    if(mode){
        ledStatusBits=(PORTE & 0x0F);
        ledStatusBits = ~ledStatusBits & 0x0F;
        isInDarkMode=1;
        PORTESET = 0x000F;    
    }
    else {
        LATECLR =  ledStatusBits;
        isInDarkMode=0;                 
    }
}
