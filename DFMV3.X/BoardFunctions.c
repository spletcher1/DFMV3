#include "GlobalIncludes.h"

unsigned char dfmID;
unsigned char isInDarkMode;
unsigned char ledStatusBits;

void InitializeBoard(){
    // Set All ID Selectors  
    SETIDSELECTOR_TRIS();
    HEARTBEAT_TRIS = 0;
    HEARTBEAT_LAT = 0; 
    GREENLED_TRIS=0;
    GREENLED_LAT=0;
    BLUELED_TRIS=0;
    BLUELED_LAT=0;
    YELLOWLED_TRIS=0;
    YELLOWLED_LAT=0;
    USERBUTTON1_PORT=0;
    USERBUTTON2_PORT=0;
    USERBUTTON1_TRIS=1;
    USERBUTTON2_TRIS=1;    
    dfmID = GETIDSELECTOR_VALUE();
}

void SetDarkMode(unsigned char mode){    
    if(mode){
        ledStatusBits=PORTE & 0x0007;
        isInDarkMode=1;
        LATECLR = 0x000F;    
    }
    else {
        PORTESET =  ledStatusBits;
        isInDarkMode=0;         
        
    }
}

