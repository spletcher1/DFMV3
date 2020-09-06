#include "GlobalIncludes.h"

unsigned char dfmID;
unsigned char isInDarkMode;
unsigned char ledStatusBits;
unsigned char usingNewPortOnly;


unsigned char GetDFMID(void){    
#ifdef PLETCHERBOARD
    return GETIDSELECTOR_VALUE();      
#endif
    
#ifdef CVBOARD
    unsigned int tmp, tmp2;
    tmp = GETIDSELECTOR_VALUE();
    tmp2 = GETID2SELECTOR_VALUE();    
    return tmp2*10+tmp;
#endif
    
}

void SetPortsStatus(void){
#ifdef PLETCHERBOARD
     usingNewPortOnly = !SWITCH_PORT;      
#endif
    
#ifdef CVBOARD
     usingNewPortOnly=1;
#endif    
    
}

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
    dfmID=GetDFMID(); 
    SetPortsStatus();   
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
