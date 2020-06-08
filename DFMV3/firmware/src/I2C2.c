#include "GlobalIncludes.h"


extern errorFlags_t volatile currentError;

void I2C2_Callback(uint32_t status){    
    if(I2C_ERROR_NONE != I2C2_ErrorGet())
    {
         currentError.bits.I2C=1;
         //YELLOWLED_ON();
    }
}

void ConfigureI2C2(void) {
    // Note: As of now, the baud rate set for the parallax RFID reader is 2400.
    // Data bits = 8; no parity; stop bits = 1;
    I2C2_CallbackRegister(I2C2_Callback,(uintptr_t)NULL);           
}

