#include "GlobalIncludes.h"

unsigned char dfmID;
unsigned char isInDarkMode;
unsigned char ledStatusBits;
unsigned char usingNewPortOnly;

void InitializeBoard(){
    // Set All ID Selectors  
    SETIDSELECTOR_TRIS();
    HEARTBEAT_TRIS = 0;
    HEARTBEAT_LAT = 1; 
    GREENLED_TRIS=0;
    GREENLED_LAT=1;
    BLUELED_TRIS=0;
    BLUELED_LAT=1;
    YELLOWLED_TRIS=0;
    YELLOWLED_LAT=1;
    USERBUTTON1_PORT=0;
    USERBUTTON2_PORT=0;
    USERBUTTON1_TRIS=1;
    USERBUTTON2_TRIS=1;    
    
    RX485_SEND_TRIS = 0;
    RX485_SEND_TRIS2 = 0;
    RX485_DISABLE_SEND();
    
    DelayMs(1000);
    dfmID = GETIDSELECTOR_VALUE();      
    usingNewPortOnly = !SWITCH_PORT; 
    if(usingNewPortOnly)
        YELLOWLED_OFF();
    else
        YELLOWLED_ON();
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

