#include "GlobalIncludes.h"

// Address Constant
#define TSL2591_ADDR  0x29

// Commands
#define TSL2591_CMD 0xA0

// Registers
#define TSL2591_CLEAR_INT  0xE7
///! Special Function Command for "Interrupt set - forces an interrupt"
#define TSL2591_TEST_INT 0xE4
#define TSL2591_ENABLE_POWEROFF 0x00    ///< Flag for ENABLE register to disable
#define TSL2591_ENABLE_POWERON 0x01    ///< Flag for ENABLE register to enable
#define TSL2591_ENABLE_AEN 0x02    ///< ALS Enable. This field activates ALS function. Writing a one activates the ALS. Writing a zero disables the ALS.
#define TSL2591_ENABLE_AIEN  0x10    ///< ALS Interrupt Enable. When asserted permits ALS interrupts to be generated, subject to the persist filter.
#define TSL2591_ENABLE_NPIEN 0x80    ///< No Persist Interrupt Enable. When asserted NP Threshold conditions will generate an interrupt, bypassing the persist filter

#define TSL2591_VISIBLE           (2)       ///< (channel 0) - (channel 1)
#define TSL2591_INFRARED          (1)       ///< channel 1
#define TSL2591_FULLSPECTRUM      (0)       ///< channel 0


#define SECONDS_IN_IDLE 5

/// TSL2591 Register map

enum {
    TSL2591_REGISTER_ENABLE = 0x00, // Enable register
    TSL2591_REGISTER_CONTROL = 0x01, // Control register
    TSL2591_REGISTER_THRESHOLD_AILTL = 0x04, // ALS low threshold lower byte
    TSL2591_REGISTER_THRESHOLD_AILTH = 0x05, // ALS low threshold upper byte
    TSL2591_REGISTER_THRESHOLD_AIHTL = 0x06, // ALS high threshold lower byte
    TSL2591_REGISTER_THRESHOLD_AIHTH = 0x07, // ALS high threshold upper byte
    TSL2591_REGISTER_THRESHOLD_NPAILTL = 0x08, // No Persist ALS low threshold lower byte
    TSL2591_REGISTER_THRESHOLD_NPAILTH = 0x09, // No Persist ALS low threshold higher byte
    TSL2591_REGISTER_THRESHOLD_NPAIHTL = 0x0A, // No Persist ALS high threshold lower byte
    TSL2591_REGISTER_THRESHOLD_NPAIHTH = 0x0B, // No Persist ALS high threshold higher byte
    TSL2591_REGISTER_PERSIST_FILTER = 0x0C, // Interrupt persistence filter
    TSL2591_REGISTER_PACKAGE_PID = 0x11, // Package Identification
    TSL2591_REGISTER_DEVICE_ID = 0x12, // Device Identification
    TSL2591_REGISTER_DEVICE_STATUS = 0x13, // Internal Status
    TSL2591_REGISTER_CHAN0_LOW = 0x14, // Channel 0 data, low byte
    TSL2591_REGISTER_CHAN0_HIGH = 0x15, // Channel 0 data, high byte
    TSL2591_REGISTER_CHAN1_LOW = 0x16, // Channel 1 data, low byte
    TSL2591_REGISTER_CHAN1_HIGH = 0x17, // Channel 1 data, high byte
} TSL2591Registers;

enum TSL2591Gain {
    Low = 0x00,
    Medium = 0x10,
    High = 0x20,
    Max = 0x30
} currentGain;

enum TSL2591Timing {
    Int100ms = 0x00,
    Int200ms = 0x01,
    Int300ms = 0x02,
    Int400ms = 0x03,
    Int500ms = 0x04,
    Int600ms = 0x05
} currentTiming;


//private const int TSL2591_STATUS_REG = 0x13;


//LOW gain: use in bright light to avoid sensor saturation
//MED: use in low light to boost sensitivity 
//HIGH: use in very low light condition

// Constants for LUX calculation
#define LUX_DF 408.0
#define LUX_COEFB 1.64  // CH0 coefficient
#define LUX_COEFC 0.59  // CH1 coefficient A
#define LUX_COEFD 0.86  //CH2 coefficient B

unsigned int lightData;
unsigned int fullLuminosity;
unsigned int visibleLuminosity;
unsigned int irLuminosity;
int TSL2591_LUX;
int tmpLUX;

unsigned char isAtMaxSensitivity;
unsigned char isAtMinSensitivity;
extern errorFlags_t volatile currentError;

extern unsigned char i2cData[10];              


unsigned char IsTSL2591Ready() {
    unsigned char found=0;   
    unsigned char data[1];    
    data[0]=TSL2591_REGISTER_DEVICE_ID | TSL2591_CMD;              
    I2C2_WriteRead(TSL2591_ADDR,&data[0],1,&found,1);
    // Make this blocking because it is only used to initialize.
    while(I2C2_IsBusy());
    if (found == 0x50)
        return 1;
    else {        
        return 0;
    }
}
void inline PowerUp() {
    unsigned char data[2];    
    data[0]=TSL2591_REGISTER_ENABLE | TSL2591_CMD;
    data[1]=TSL2591_ENABLE_POWERON;   
    I2C2_Write(TSL2591_ADDR,&data[0],2);   
    // Make this blocking because it is called only at the beginning.
    while(I2C2_IsBusy());
        
}

void RequestTimingAndGainChangeCall() {
    unsigned char data[2];
    data[0]=TSL2591_REGISTER_CONTROL | TSL2591_CMD;
    data[1]=(unsigned char) currentGain | (unsigned char) currentTiming;
    I2C2_Write(TSL2591_ADDR,&data[0],2);    
}

void inline Enable() {
    unsigned char data[2];
    data[0]=TSL2591_REGISTER_ENABLE | TSL2591_CMD;
    data[1]=TSL2591_ENABLE_POWERON | TSL2591_ENABLE_AEN;
    I2C2_Write(TSL2591_ADDR,&data[0],2);     
    while(I2C2_IsBusy());
}

unsigned char ConfigureTSL2591() {
    PowerUp();
    DelayMs(10);
    Enable();
    DelayMs(10);
    if (!IsTSL2591Ready()) {
        return 0;
    }  
    DelayMs(10);
    currentGain=Medium;
    currentTiming=Int300ms;  
    RequestTimingAndGainChangeCall();
    while(I2C2_IsBusy());
    return 1;
}

void inline Disable() {
    unsigned char data[2];
    data[0]=TSL2591_REGISTER_ENABLE | TSL2591_CMD;
    data[1]=TSL2591_ENABLE_POWERON;
    I2C2_Write(TSL2591_ADDR,&data[0],2);           
    while(I2C2_IsBusy());
}

void inline PowerDown() {
   unsigned char data[2];
    data[0]=TSL2591_REGISTER_ENABLE | TSL2591_CMD;
    data[1]=TSL2591_ENABLE_POWEROFF;
    I2C2_Write(TSL2591_ADDR,&data[0],2);     
    while(I2C2_IsBusy());
}
// Need to wait for at least 200ms after calling StartReading to get these data.

unsigned char GetTSL2591Status() {
    unsigned char found;
    unsigned char data[1];    
    Enable();    
    
    data[0]=TSL2591_REGISTER_DEVICE_STATUS | TSL2591_CMD;              
    I2C2_WriteRead(TSL2591_ADDR,&data[0],1,&found,1);
    while(I2C2_IsBusy());
    Disable();
    return found;

}

unsigned char CheckTimingAndGain(){
    if (currentGain != (i2cData[0] & 0xF0)) {
        return 0;
    }
    if (currentTiming != (i2cData[0] & 0x0F)) {        
        return 0;        
    }           
    return 1;
}

void RequestTimingAndGainCall() {
    unsigned char regData = TSL2591_REGISTER_CONTROL | TSL2591_CMD;         
    i2cData[0]=i2cData[1]=i2cData[2]=i2cData[3]=0;
    I2C2_WriteRead(TSL2591_ADDR,&regData,1,&i2cData[0],1);    
    Delay10us(5);
}

void StoreFullLuminosity(){
    // Just to make sure...it shouldn't be busy    
    fullLuminosity=(i2cData[3]<<24) + (i2cData[2]<<16) + (i2cData[1]<<8) + i2cData[0];
    // Need to swap byte order to get luminosity.   
    irLuminosity = fullLuminosity >> 16;
    visibleLuminosity = (fullLuminosity - irLuminosity);    
}

void RequestFullLuminosity() {    
    unsigned char reg = TSL2591_CMD | TSL2591_REGISTER_CHAN0_LOW;    
    i2cData[0]=i2cData[1]=i2cData[2]=i2cData[3]=0;
    I2C2_WriteRead(TSL2591_ADDR,&reg,1,&i2cData[0],4); 
    Delay10us(5);
}

void IncreaseSensitivity() {
    // Only choose a range of sensitivities to avoid so many combinations.
    if (currentTiming == Int100ms) {
        currentTiming = Int300ms;        
    } else if (currentTiming == Int300ms) {
        currentTiming = Int500ms;          
    } else if (currentGain == Low) {
        currentGain =Low;        
    } else if (currentGain == Medium) {
        currentGain=High;        
    }       
    if (currentTiming == Int500ms && currentGain == High) {
        isAtMaxSensitivity = 1;
        isAtMinSensitivity = 0;
    } else if (currentTiming == Int100ms && currentGain == Low) {
        isAtMinSensitivity = 1;
        isAtMaxSensitivity = 0;
    } else {
        isAtMinSensitivity = 0;
        isAtMaxSensitivity = 0;
    }
}

void DecreaseSensitivity() {
    if (currentGain == High) {
        currentGain=Medium;        
    } else if (currentGain == Medium) {
        currentGain=Low;        
    } else if (currentTiming == Int500ms) {
        currentTiming=Int300ms;        
    } else if (currentTiming == Int300ms) {
        currentTiming=Int100ms;       
    }
    if (currentTiming == Int500ms && currentGain == High) {
        isAtMaxSensitivity = 1;
        isAtMinSensitivity = 0;
    } else if (currentTiming == Int100ms && currentGain == Low) {
        isAtMinSensitivity = 1;
        isAtMaxSensitivity = 0;
    } else {
        isAtMinSensitivity = 0;
        isAtMaxSensitivity = 0;
    }
}

void GetLux() {
    float lux, d0, d1, lux1, lux2, cpl, again, atime;

    atime = (float) ((float) currentTiming + 1) * 100;
    switch (currentGain) {
        case Low: again = 1.0F;
            break;
        case Medium: again = 25.0F;
            break;
        case High: again = 428.0F;
            break;
        case Max: again = 9876.0F;
            break;
        default: again = 1.0F;
            break;
    }

    unsigned int CH0 = fullLuminosity & 0xFFFF;
    unsigned int CH1 = fullLuminosity >> 16;

    if (CH0 + CH1 == 0) { // This implies the board was unplugged or an error occured.       
        tmpLUX = -1;
        return;
    }
    // Determine if either sensor saturated (0xFFFF)
    // If so, abandon ship (calculation will not be accurate)
    if ((CH0 > 45000) || (CH1 > 45000)) // I set this at 40000 (out of 65535) to turn down sensitivity.
    {
        if (!isAtMinSensitivity) {
            DecreaseSensitivity();
        }
    } else if ((CH0 + CH1) < 30) {
        if (!isAtMaxSensitivity) {
            IncreaseSensitivity();
        }
    }


    // Convert from unsigned integer to floating point
    d0 = (float) CH0;
    d1 = (float) CH1;

    cpl = (atime * again) / LUX_DF;
    lux1 = (d0 - (LUX_COEFB * d1)) / cpl;
    lux2 = ((LUX_COEFC * d0) - (LUX_COEFD * d1)) / cpl;
    lux = lux1 > lux2 ? lux1 : lux2;

    tmpLUX = (int) lux;
}

/*
void StepTSL2591() {
    if (isTSL2591Configured == 0) return;
    switch (currentState_TSL) {
        case RequestLuminosity:            
            RequestFullLuminosity();                                    
            break;
        case Idle:
            if (idleCounter_tsl++ >= SECONDS_IN_IDLE) {                             
                currentState_TSL = RequestTimingAndGain;
                idleCounter_tsl = 0;
            }
            break;
        case RequestTimingAndGainChange:
            RequestTimingAndGainChangeCall();
            break;
        case RequestTimingAndGain:
            RequestTimingAndGainChangeCall();
            break;
        case LuxCalculation:
            GetLux();         
            currentState_TSL = LuxReady;
            break;                    
        case LuxReady:
            if(currentError.bits.I2C==1 || currentError.bits.TSL2591==1)
                TSL2591_LUX = 0;
            else
                TSL2591_LUX = tmpLUX;
            currentState_TSL = Idle;
            break;
        default:
            break;
    }

}*/

