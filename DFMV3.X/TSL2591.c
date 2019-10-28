#include "GlobalIncludes.h"

// Address Constant
#define TSL2591_ADDR  0x29
// This is the actual address used for I2C communication
// it is 0x29 shifted one to left to leave LSB open.
#define TSL2591_ADDR_ADJ  0x52
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


#define SECONDS_IN_IDLE 2

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

enum TSL2591State {
    Reading,
    LuxCalculation,
    LuxReady,
    Idle,
} currentState_TSL;

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
unsigned char isTSL2591Configured;
unsigned char didSensitivityChange;
unsigned char isAtMaxSensitivity;
unsigned char isAtMinSensitivity;
int idleCounter_tsl;

unsigned char IsTSL2591Ready() {
    unsigned char found;
    I2C_RESULT result;
    result = Read8FromI2C2(TSL2591_ADDR_ADJ, TSL2591_REGISTER_DEVICE_ID | TSL2591_CMD, &found);
    if (found == 0x50 && result == I2C_SUCCESS)
        return 1;
    else
        return 0;
}

void SetTimingAndGain(enum TSL2591Gain gain, enum TSL2591Timing timing) {
    Write8ToI2C2(TSL2591_ADDR_ADJ, TSL2591_REGISTER_CONTROL | TSL2591_CMD, (unsigned char) gain | (unsigned char) timing);
    currentGain = gain;
    currentTiming = timing;
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

void inline Enable() {
    Write8ToI2C2(TSL2591_ADDR_ADJ, TSL2591_REGISTER_ENABLE | TSL2591_CMD, TSL2591_ENABLE_POWERON | TSL2591_ENABLE_AEN); // Power up.    
}

void inline Disable() {
    Write8ToI2C2(TSL2591_ADDR_ADJ, TSL2591_REGISTER_ENABLE | TSL2591_CMD, TSL2591_ENABLE_POWERON); // Power up.
}

void inline PowerUp() {
    Write8ToI2C2(TSL2591_ADDR_ADJ, TSL2591_REGISTER_ENABLE | TSL2591_CMD, TSL2591_ENABLE_POWERON); // Power up.
}

void inline PowerDown() {
    Write8ToI2C2(TSL2591_ADDR_ADJ, TSL2591_REGISTER_ENABLE | TSL2591_CMD, TSL2591_ENABLE_POWEROFF); // Power up.
}
// Need to wait for at least 200ms after calling StartReading to get these data.

unsigned char GetTSL2591Status(){  
  unsigned char result, found;
  Enable();
  result = Read8FromI2C2(TSL2591_ADDR_ADJ, TSL2591_REGISTER_DEVICE_STATUS | TSL2591_CMD, &found);   
  Disable();
  return found;
    
}

unsigned char CheckTimingAndGain(){  
  unsigned char result, regData;
  Enable();
  result = Read8FromI2C2(TSL2591_ADDR_ADJ, TSL2591_REGISTER_CONTROL | TSL2591_CMD, &regData); 
  Disable();
  if(currentGain != (regData & 0xF0))
      return 0;
  if(currentTiming != (regData & 0x0F))
      return 0;
  return 1;    
}

void GetFullLuminosity() {
    unsigned char status;
    status = GetTSL2591Status();            
    Read32FromI2C2Backward(TSL2591_ADDR_ADJ, TSL2591_CMD | TSL2591_REGISTER_CHAN0_LOW, &fullLuminosity);    
    // Need to swap byte order to get luminosity.   
    irLuminosity = fullLuminosity >> 16;
    visibleLuminosity = (fullLuminosity - irLuminosity);    
}

void IncreaseSensitivity() {
    // Only choose a range of sensitivities to avoid so many combinations.
    if (currentTiming == Int100ms) {
        SetTimingAndGain(currentGain, Int300ms);
        didSensitivityChange=1;
    } else if (currentTiming == Int300ms) {
        SetTimingAndGain(currentGain, Int500ms);
        didSensitivityChange=1;
    } else if (currentGain == Low) {
        SetTimingAndGain(Medium, currentTiming);
        didSensitivityChange=1;
    } else if (currentGain == Medium) {
        SetTimingAndGain(High, currentTiming);
        didSensitivityChange=1;
    }
}

void DecreaseSensitivity() {
    if (currentGain == High) {
        SetTimingAndGain(Medium, currentTiming);
        didSensitivityChange=1;
    } else if (currentGain == Medium) {
        SetTimingAndGain(Low, currentTiming);
        didSensitivityChange=1;
    } else if (currentTiming == Int500ms) {
        SetTimingAndGain(currentGain, Int300ms);
        didSensitivityChange=1;
    } else if (currentTiming == Int300ms) {
        SetTimingAndGain(currentGain, Int100ms);
        didSensitivityChange=1;
    }
}

void GetLux() {
    float lux, d0, d1, lux1, lux2, cpl, again, atime;
   
    if(!CheckTimingAndGain()){
        SetTimingAndGain(currentGain,currentTiming);
        tmpLUX=-1;
        return;
    }
    
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

unsigned char ConfigureTSL2591() {       
    PowerUp();
    DelayMs(10);    
    if (!IsTSL2591Ready()) {
        isTSL2591Configured = 0;
        return 0;
    }
    isTSL2591Configured=1;
    DelayMs(10);    
    SetTimingAndGain(Medium, Int300ms);    
    didSensitivityChange=0;
    idleCounter_tsl = SECONDS_IN_IDLE; // This will force a first measure right away.
    currentState_TSL=Idle;
    return 1;
}

// This should probably step once every second.
void StepTSL2591() { 
    if(isTSL2591Configured==0) return;
    switch(currentState_TSL){
        case Reading:
            GetFullLuminosity();
            Disable();
            currentState_TSL=LuxCalculation;
            break;
        case Idle:
            if(idleCounter_tsl++>=SECONDS_IN_IDLE){
                Enable();
                currentState_TSL = Reading;
                idleCounter_tsl=0;
            }
            break;
        case LuxCalculation:           
            GetLux();
            if(didSensitivityChange) {
                currentState_TSL=Idle;
                didSensitivityChange=0;
            }
            else
                currentState_TSL=LuxReady;
            break;
        case LuxReady:
            TSL2591_LUX=tmpLUX;
            currentState_TSL=Idle;
            break;
        default:
            break;
    }
    
}
