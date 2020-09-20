// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "GlobalIncludes.h"

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

errorFlags_t volatile currentError;
extern enum PacketState currentPacketState;
extern unsigned char volatile timerFlag_1ms;
extern unsigned char volatile timerFlag_1sec;
extern unsigned char volatile analogUpdateFlag;
extern unsigned char volatile timerFlag_200ms;

void InitializeRun(){ 
    InitializeBoard(); 
    currentError.byte=0x00;  
    ConfigureUpdateTimer();
    ConfigureAnalogInputs();
    ConfigureUART2();        
    ConfigureI2C2();
    ConfigureButtons();      
    InitializeLEDControl(0,0,0);    
    InitializeStatusPacketBuffer();     
    ConfigureOpto();    
}




int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );         
    InitializeRun();    
    while ( true )
    {         
        if (timerFlag_1ms) {             
            ProcessButtonStep();
            //StepLEDControl();           
            StepUART();
            timerFlag_1ms = 0;          
        }
        if(timerFlag_200ms){
             StepPacketManager();
             timerFlag_200ms = 0;  
        }
        if(timerFlag_1sec){         
#ifdef ENABLEI2C            
            StepI2C();           
#endif
            timerFlag_1sec=0;
        }      
        if(analogUpdateFlag){            
            StepADC();                   
            analogUpdateFlag=0;            
        }
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */
    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

