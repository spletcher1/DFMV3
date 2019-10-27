/******************************************************************************/
/*  Files to Include                                                          */
/******************************************************************************/

#include "GlobalIncludes.h"         /* System funct/params, like osc/periph config    */
#include "SP_ConfigP32.h"           /* User funct/params, such as InitApp             */
#include <stdio.h>

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


extern unsigned char isPacketReceived;
extern unsigned char isStatusRequested;
extern unsigned char dfmID;
extern struct StatusPacket currentStatus;
extern unsigned int TSL2591_LUX;
extern float Si7021_Humidity;
extern float Si7021_Temperature;

// TODO: Figure out what to make this thing do.  It is essentially not
// doing anything once the continuous sampling routine is working.
int32_t main(void) {   
    unsigned char test;
    char st[100];
    Startup();
    InitializeBoard();     
    ConfigureUpdateTimer();
    ConfigureUART1();    
    //ConfigureOpto(); 
    ConfigureI2C2();    
    DelayMs(1000);   
    sprintf(st,"Hello");
    StringToUART1(st);
    
    // Maybe add error condition here?
    //test=ConfigureTSL2591();  
    //if(test==0){
    //    myprintf("TSL2591 not Configured!");
    //}
    test=ConfigureSi7021();
     if(test==0){
        //myprintf("Si7021 not Configured!");
    }
    //StartContinuousSampling();       
    DelayMs(1000);     
    while (1) {  
        //StepTSL2591();
        StepSi7021();
        DelayMs(1000);  
        sprintf(st,"H = %g    T = %g\r\n",(double)Si7021_Humidity,(double)Si7021_Temperature);
        StringToUART1(st);
        //myprintf("H = %f\r\n",Si7021_Humidity);
        //myprintf("LUX = %d\r\n",TSL2591_LUX);
        
        //if(isPacketReceived){
        //    ProcessPacket();
        //    isPacketReceived=0;
            //FLIP_GREEN_LED();
            //FLIP_BLUE_LED();
            //FLIP_YELLOW_LED();
        }        
}



