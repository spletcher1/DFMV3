#ifndef __GLOBALINCLUDES_H
#define __GLOBALINCLUDES_H

#include "HardwareProfile.h"
#include "UART2.h"
#include "definitions.h"                // SYS function prototypes
#include "BoardFunctions.h"
#include "Delays.h"
#include "UpdateTimer.h"
#include "definitions.h"  
#include "AnalogInputs.h"
#include "PacketManager.h"
#include "OptoFunctions.h"
#include "LEDControl.h"
#include "Buttons.h"



typedef union {
    unsigned char byte;
    struct ErrorBits {
        unsigned I2C : 1;
        unsigned IDERR : 1;
        unsigned PACKETTYPEERR : 1;
        unsigned DMA_TX : 1;
        unsigned DMA_RX : 1;
        unsigned PACKETSIZEERR : 1;
        unsigned AIINTERRUPT : 1;
        unsigned OINTERRUPT : 1;
    } bits;
} errorFlags_t;

//#include <math.h>
//#define DEBUG_UART1


#endif