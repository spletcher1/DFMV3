#include "GlobalIncludes.h"

extern errorFlags_t volatile currentError;
extern unsigned char waitingOnSIRequest;
extern enum TSL2591State currentState_TSL;
extern unsigned char tslData[4];        

void I2C2_Callback(uint32_t status){ 
    I2C_ERROR tmp = I2C2_ErrorGet();
    if(tmp == I2C_ERROR_NACK && waitingOnSIRequest==1){
        return;
    }
    
    if (currentState_TSL != Idle){
        switch(currentState_TSL){
            case RequestLuminosity:
                StoreFullLuminosity();
                break;
            case RequestTimingAndGain:
                CheckTimingAndGain();
                break;
            case RequestTimingAndGainChange:
                TimingAndGainChangeComplete();
                break;
            case LuxCalculation:
                break;
            case LuxReady:
                break;
            case Idle:
                break;
        }   
    }
    
    else if(I2C_ERROR_NONE != tmp)
    {
         currentError.bits.I2C=1;        
    }
}

void ConfigureI2C2(void) {
    // Note: As of now, the baud rate set for the parallax RFID reader is 2400.
    // Data bits = 8; no parity; stop bits = 1;
    I2C2_CallbackRegister(I2C2_Callback,(uintptr_t)NULL);           
}