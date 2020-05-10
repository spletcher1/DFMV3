/******************************************************************************/
/*  Files to Include                                                          */
/******************************************************************************/

#include "GlobalIncludes.h"         /* System funct/params, like osc/periph config    */
#include "SP_ConfigP32.h"           /* User funct/params, such as InitApp             */

errorFlags_t volatile currentError;
extern enum PacketState currentPacketState;
extern unsigned char volatile timerFlag_1ms;
extern unsigned char volatile timerFlag_100ms;
extern unsigned char volatile timerFlag_1sec;
extern unsigned char volatile analogUpdateFlag;


extern unsigned int TSL2591_LUX;
extern unsigned int Si7021_Humidity;
extern unsigned int Si7021_Temperature;



void InitializeRun(){
    Startup();
    InitializeBoard();
    DelayMs(500);
    currentError.byte=0x00;      
    ConfigureUpdateTimer();
    ConfigureUART2();    
    ConfigureOpto();
    FreeI2C();
    ConfigureI2C2();
    ConfigureButtons();
   
    DelayMs(100);
    if(ConfigureTSL2591()==0){
        currentError.bits.TSL2591=1;
    }
    DelayMs(50);
    if(ConfigureSi7021()==0){
        currentError.bits.Si7021=1;
       }
    InitializeLEDControl(0,0,0);    
    InitializeStatusPacketBuffer();   
    StartContinuousSampling();    
}



int32_t main(void) {
    int i;
    InitializeRun();
    
    //for(i=0;i<12;i++)
        //CurrentValues[i]=(i*5*128);
    //    CurrentValues[i]=(3*128);
    
    //CurrentValues[2] = 625*128;
    //CurrentValues[8] = 300*128;
    DelayMs(100);
    //counter=0;
    while (1) {         
         if (currentPacketState==Complete) {
            ProcessPacket();
            currentPacketState = None;        
        }
        if(analogUpdateFlag){
            StepADC();                   
            analogUpdateFlag=0;            
        }
        if (timerFlag_1sec) {    
            //StringToUART2("One Second!\n\r");
            //myprintf("1 = %d   2 = %d\n", CurrentValues[0],CurrentValues[1]);            
            //myprintf("(%d) T = %d   H = %d   L = %d\r", counter++, Si7021_Temperature,Si7021_Humidity,TSL2591_LUX);            
            //myprintf("(%d) L = %d\n\r", counter++,TSL2591_LUX);        
            if(currentError.bits.I2C==0){                               
                StepTSL2591();
                StepSi7021();                   
            }
            timerFlag_1sec = 0;               
        }             
        if (timerFlag_100ms) {
            StepPacketManager();
            timerFlag_100ms = 0;
        }
        if (timerFlag_1ms) {            
            ProcessButtonStep();
            StepLEDControl();
            timerFlag_1ms = 0;            
        }
            
       
    }
}

