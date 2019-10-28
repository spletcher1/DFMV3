/******************************************************************************/
/*  Files to Include                                                          */
/******************************************************************************/

#include "GlobalIncludes.h"         /* System funct/params, like osc/periph config    */
#include "SP_ConfigP32.h"           /* User funct/params, such as InitApp             */


/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/

/* i.e. uint32_t <variable_name>; */

/******************************************************************************/
/* Main Program  Slave V 0.2                                                             */
/******************************************************************************/
// Don't forget to set the definitions for I2C and Analog sampling
// in the appropriate header files.

// This function takes about 580us.



errorFlags_t currentError;
extern unsigned char volatile isPacketReceived;
extern unsigned char volatile timerFlag_1ms;
extern unsigned char volatile timerFlag_100ms;
extern unsigned char volatile timerFlag_1sec;
//extern unsigned char isStatusRequested;
//extern unsigned char dfmID;
//extern struct StatusPacket currentStatus;
//extern unsigned int TSL2591_LUX;
//extern unsigned int Si7021_Humidity;
//extern unsigned int Si7021_Temperature;

// TODO: Figure out what to make this thing do.  It is essentially not
// doing anything once the continuous sampling routine is working.

int32_t main(void) {
    Startup();
    InitializeBoard();
    currentError.byte=0x00;   
    ConfigureUpdateTimer();
    ConfigureUART1();
    ConfigureOpto();
    ConfigureI2C2();
    DelayMs(50);
    if(ConfigureTSL2591()==0){
        currentError.bits.CONFIGURATION=1;
    }
    DelayMs(50);
    if(ConfigureSi7021()==0){
        currentError.bits.CONFIGURATION=1;
    }
    //StartContinuousSampling();       
    DelayMs(100);
    
    while (1) {
        if (timerFlag_1sec) {
            StepTSL2591();
            StepSi7021();
            timerFlag_1sec = 0;
            FLIP_GREEN_LED();
        }

        if (timerFlag_100ms) {
            timerFlag_100ms = 0;
            FLIP_BLUE_LED();
        }

        if (timerFlag_1ms) {
            timerFlag_1ms = 0;
            FLIP_YELLOW_LED();
        }

        //myprintf("H = %d   T = %d   L = %d\r\n",Si7021_Humidity,Si7021_Temperature,TSL2591_LUX);    
        if (isPacketReceived) {
            ProcessPacket();
            isPacketReceived = 0;
        }
    }
}

