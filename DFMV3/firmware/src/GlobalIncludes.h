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
#include "I2C2.h"
#include "TSL2591.h"
#include "Si7021.h"



typedef union {
    unsigned char byte;
    struct ErrorBits {
        unsigned I2C : 1;
        unsigned OERR : 1;
        unsigned PACKET : 1;
        unsigned Si7021 : 1;
        unsigned TSL2591 : 1;
        unsigned FERR : 1;
        unsigned INTERRUPT : 1;
        unsigned PERR : 1;
    } bits;
} errorFlags_t;

//#include <math.h>
//#define DEBUG_UART1


#endif