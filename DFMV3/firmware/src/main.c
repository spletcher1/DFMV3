/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

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
    
    //FreeI2C();
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
    YELLOWLED_OFF();
    BLUELED_OFF();    
    while ( true )
    {         
        if (timerFlag_1ms) {             
            ProcessButtonStep();
            StepLEDControl();           
            StepUART();
            timerFlag_1ms = 0;          
        }
        if(timerFlag_200ms){
             StepPacketManager();
             timerFlag_200ms = 0;  
        }
        if(timerFlag_1sec){           
            StepI2C();            
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

