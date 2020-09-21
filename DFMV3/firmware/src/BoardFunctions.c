#include "GlobalIncludes.h"

unsigned char dfmID;
unsigned char isInDarkMode;
unsigned char ledStatusBits;
unsigned char usingNewPort;


unsigned char GetDFMID(void){    
#ifdef PLETCHERBOARD
    return GETIDSELECTOR_VALUE();      
#endif
    
#ifdef CVBOARD
    unsigned int tmp,tmp2;
    tmp = GETIDSELECTOR_VALUE();
    tmp2 = GETID2SELECTOR_VALUE(); 
    // These are needed to account for the pins being pulled up by default.
    // Otherwise the switches would return 10 when set to zero.
    if(tmp==10) tmp=0;
    if(tmp2==10) tmp2=0;    
    return tmp2*10+tmp;
#endif
    
}

void SetPortsStatus(void){
#ifdef PLETCHERBOARD
     usingNewPort = 0;      
#endif    
#ifdef CVBOARD
     usingNewPort=1;
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
    if(usingNewPort)
        YELLOWLED_OFF();
    else
        YELLOWLED_ON();
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
        if(usingNewPort)
            YELLOWLED_OFF();
        else
            YELLOWLED_ON();
        isInDarkMode=0;                 
    }
}

void ToggleDarkMode(){
    if(isInDarkMode)
        SetDarkMode(0);
    else
        SetDarkMode(1);
}
