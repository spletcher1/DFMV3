/******************************************************************************/
/*  Files to Include                                                          */
/******************************************************************************/
#ifdef __XC32
#include <xc.h>          /* Defines special funciton registers, CP0 regs  */
#endif

#include <plib.h>           /* Include to use PIC32 peripheral libraries      */
#include <stdint.h>         /* For uint32_t definition                        */
#include <stdbool.h>        /* For true/false definition                      */

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

void Delay10us(DWORD dwCount)
{
	volatile DWORD _dcnt;

	_dcnt = dwCount*((DWORD)(0.00001/(1.0/GetInstructionClock())/10));
	while(_dcnt--)
	{
		#if defined(__C32__)
			Nop();
			Nop();
			Nop();
		#endif
	}
}

void DelayMs(WORD ms)
{
    unsigned char i;
    while(ms--)
    {
        i=4;
        while(i--)
        {
            Delay10us(25);
        }
    }
}

void ConfigureBoard(){
    HEARTBEAT_TRIS = 0;
    SIGNAL_LED_TRIS = 0;
    YELLOW_LED_TRIS=0;
    BLUE_LED_TRIS=0;
    TIMER_PIN_TRIS=0;
    
    HEARTBEAT_LED_OFF();
    SIGNAL_LED_OFF();
    BLUE_LED_OFF();
    YELLOW_LED_OFF();
    TIMER_PIN_OFF();
        
    SWITCH_TRIS=1;
    BUTTON1_TRIS=1;
    BUTTON2_TRIS=1;
    
    ID_SWITCH1_TRIS=1;
    ID_SWITCH2_TRIS=1;
    ID_SWITCH4_TRIS=1;
    ID_SWITCH8_TRIS=1;
            
}


// TODO: Figure out what to make this thing do.  It is essentially not
// doing anything once the continuous sampling routine is working.
int32_t main(void) {
    Startup();
    ConfigureBoard();
 
    ClearAnalogValues();
    ConfigureUpdateTimer();
    ConfigureOpto();
    DelayMs(1000);   
    
    //I2C2_Configure(); // THIS NEED REWRITING FOR V3 BOARD.  USED TO GET ENVIRONMENTAL INFO
    StartContinuousSampling();
    ConfigureDefaultUART3();
    StringToUART3("UART Here\r");          
    while (1) {        
        //if(ShowOutput) {
        //    GetNewAverages();
        //     myprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r", SPECIFIC_SLAVE_ID, CurrentValues[0], CurrentValues[1], CurrentValues[2], CurrentValues[3],
        //        CurrentValues[4], CurrentValues[5], CurrentValues[6], CurrentValues[7], CurrentValues[8], CurrentValues[9],
        //        CurrentValues[10], CurrentValues[11],CurrentValues[12]);
        //     ShowOutput=0;
        //}
    }
}



