#include "GlobalIncludes.h"

extern errorFlags_t volatile currentError;
enum I2CState currentState_I2C;
unsigned char i2cData[10];        
int idleCounter;
unsigned char isSi7021Configured = 0;
unsigned char isTSL2591Configured;

extern int TSL2591_LUX;
extern int tmpLUX;
extern unsigned int Si7021_Humidity;
extern unsigned int Si7021_Temperature;

#define SECONDS_IN_IDLE 5


void I2C2_Callback(uint32_t status){ 
    I2C_ERROR tmp = I2C2_ErrorGet();             
    if(I2C_ERROR_NONE != tmp)
    {
        currentError.bits.I2C=1;   
        return;
    }

    if (currentState_I2C != Idle){
        switch(currentState_I2C){
            case RequestLuminosity:
                StoreFullLuminosity();
                currentState_I2C = LuxCalculation;
                break;
            case RequestTimingAndGain:
                if(CheckTimingAndGain())
                    currentState_I2C = RequestLuminosity;
                else
                     currentState_I2C =RequestTimingAndGainChange;
                break;
            case RequestTimingAndGainChange:
                currentState_I2C = RequestLuminosity;                
                break;                
            case LuxCalculation:
                break;
            case LuxReady:
                break;
            case RequestMeasure:                                         
                UpdateHumidity(); 
                currentState_I2C = GetTemperature;
                break;          
            case GetTemperature:
                UpdateTemperature();
                currentState_I2C = Calculate;
                break;
            case Calculate:
                break;                
            case Idle:
                break;
        }   
    }
         
}

void ConfigureI2C2(void) {
    // Note: As of now, the baud rate set for the parallax RFID reader is 2400.
    // Data bits = 8; no parity; stop bits = 1;
    I2C2_CallbackRegister(I2C2_Callback,(uintptr_t)NULL);   
    idleCounter=0;
    currentState_I2C=Idle;    
    if((isTSL2591Configured=ConfigureTSL2591())==0){
        currentError.bits.TSL2591=1;      
    }
    DelayMs(100);
    if((isSi7021Configured=ConfigureSi7021())==0){
        currentError.bits.Si7021=1;        
    }
    
}


void StepI2C() {
    if (isTSL2591Configured == 0 || isSi7021Configured == 0) return;
    if (currentError.bits.I2C==1) return;
    switch (currentState_I2C) {
        case RequestLuminosity:            
            RequestFullLuminosity();                                    
            break;
        case Idle:
            if (idleCounter++ >= SECONDS_IN_IDLE) {                             
                currentState_I2C = RequestTimingAndGain;
                idleCounter = 0;
            }
            break;
        case RequestTimingAndGainChange:
            RequestTimingAndGainChangeCall();
            break;
        case RequestTimingAndGain:
            RequestTimingAndGainCall();
            break;
        case LuxCalculation:
            GetLux();         
            currentState_I2C = LuxReady;
            break;                    
        case LuxReady:
            if(currentError.bits.I2C==1 || currentError.bits.TSL2591==1)
                TSL2591_LUX = 0;
            else
                TSL2591_LUX = tmpLUX;
            currentState_I2C = RequestMeasure;
            break;
        case RequestMeasure:
            RequestTempHumidityMeasure();            
            break;      
        case GetTemperature:
            GetTemperatureData();        
            break;     
        case Calculate:
            if(currentError.bits.I2C==1 || currentError.bits.Si7021==1){
                Si7021_Temperature = 0;
                Si7021_Humidity =0;
            }
            else
                UpdateTempAndHumidity();
            currentState_I2C = Idle;
            break;
        default:
            break;                        
    }

}


