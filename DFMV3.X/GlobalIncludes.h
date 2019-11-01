#ifndef __GLOBALINCLUDES_H
#define __GLOBALINCLUDES_H

#include <p32xxxx.h>
#include <xc.h>
#include <peripheral/int.h>
#include <plib.h>
#include "uart.h"
#include "timer.h"
#include "system.h"
#include "adc10.h"
#include "HardwareProfile.h"
#include "SystemFunctions.h"
#include "UART1.h"
#include "UpdateTimer.h"
#include "I2C2_Master.h"
#include "AnalogInputs.h"
#include "OptoFunctions.h"
#include "BoardFunctions.h"
#include "Buttons.h"
#include <stdio.h>

typedef union {
    unsigned char byte;
    struct ErrorBits {
        unsigned I2C : 1;
        unsigned UART : 1;
        unsigned PACKET : 1;
        unsigned Si7021 : 1;
        unsigned TSL2591 : 1;
        unsigned CONFIGURATION : 1;
        unsigned TBD2 : 1;
        unsigned TBD3 : 1;
    } bits;
} errorFlags_t;

//#include <math.h>
//#define DEBUG_UART1


#endif