/******************************************************************************/
/*  Files to Include                                                          */
/******************************************************************************/

#include "GlobalIncludes.h"         /* System funct/params, like osc/periph config    */
#include "SP_ConfigP32.h"           /* User funct/params, such as InitApp             */

errorFlags_t currentError;
extern unsigned char volatile isPacketReceived;
extern unsigned char volatile timerFlag_1ms;
extern unsigned char volatile timerFlag_100ms;
extern unsigned char volatile timerFlag_1sec;

extern int volatile CurrentValues[12];



int32_t main(void) {
    int i;
    Startup();
    InitializeBoard();
    currentError.byte=0x00;   
    ConfigureUpdateTimer();
    ConfigureUART1();
    ConfigureOpto();
    ConfigureI2C2();
    ConfigureButtons();
    DelayMs(50);
    if(ConfigureTSL2591()==0){
        currentError.bits.CONFIGURATION=1;
    }
    DelayMs(50);
    if(ConfigureSi7021()==0){
        currentError.bits.CONFIGURATION=1;
    }
    //StartContinuousSampling();    
    for(i=0;i<12;i++)
        CurrentValues[i]=(i*5*128);
    DelayMs(100);
    
    while (1) {
        if (timerFlag_1sec) {
            StepTSL2591();
            StepSi7021();
            timerFlag_1sec = 0;            
        }

        if (timerFlag_100ms) {
            timerFlag_100ms = 0;
        }

        if (timerFlag_1ms) {
            ProcessButtonStep();
            timerFlag_1ms = 0;            
        }

        //myprintf("H = %d   T = %d   L = %d\r\n",Si7021_Humidity,Si7021_Temperature,TSL2591_LUX);    
        if (isPacketReceived) {
            ProcessPacket();
            isPacketReceived = 0;
        }
    }
}

