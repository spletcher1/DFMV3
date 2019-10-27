#include "GlobalIncludes.h"

#define Si7021_address                          0x40
#define Si7021_address_adj                      0x80

#define Si7021_Measure_RH_Hold_Mode             0xE5
#define Si7021_Measure_RH_No_Hold_Mode          0xF5
#define Si7021_Measure_T_Hold_Mode              0xE3
#define Si7021_Measure_T_No_Hold_Mode           0xF3
#define Si7021_Read_T_from_Last_RH_Value        0xE0
#define Si7021_Reset                            0xFE
#define Si7021_Write_Config_Register            0xE6
#define Si7021_Read_Config_Register             0xE7
#define Si7021_Write_Heater_Control_Register    0x51
#define Si7021_Read_Heater_Control_Register     0x11
#define Si7021_Read_ID1_H                       0xFA
#define Si7021_Read_ID1_L                       0x0F
#define Si7021_Read_ID2_H                       0xFC
#define Si7021_Read_ID2_L                       0xC9
#define Si7021_Read_Firmware_Version_H          0x84
#define Si7021_Read_Firmware_Version_L          0xB8

#define Si7021_Config_Resolution_RH_12_T14      0x00
#define Si70211_Config_Resolution_RH_8_T12      0x01
#define Si70211_Config_Resolution_RH_10_T13     0x80
#define Si70211_Config_Resolution_RH_11_T11     0x81

#define Si70211_Config_VDD_Ok                   0x00
#define Si70211_Config_VDD_Low                  0x40

#define Si70211_Config_Heater_Off               0x00
#define Si70211_Config_Heater_On                0x04


#define SECONDS_IN_IDLE 60

enum Si7021State {
    Measuring,
    Calculation,
    GettingTemperature,    
    Idle,
} currentState;

float Si7021_Humidity;
float Si7021_Temperature;
unsigned int tmpHumidity,tmpTemperature;
unsigned char isSi7021Configured=0;
int idleCounter;

unsigned char IsSi7021Ready() {
    unsigned char found;
    I2C_RESULT result;
    result = Read8FromI2C2(Si7021_address_adj, Si7021_Read_Config_Register, &found);
    if (found != 0x00 && result == I2C_SUCCESS)
        return 1;
    else
        return 0;
}

unsigned char ConfigureSi7021(){
    Si7021_Humidity=Si7021_Temperature=tmpHumidity=tmpTemperature=0;
    idleCounter = 0;
    DelayMs(2);    
    if (!IsSi7021Ready()) {
        isSi7021Configured = 0;
        return 0;
    }
    isSi7021Configured=1;
    currentState = Idle;
    return 1;
}

void CallForTempHumidityMeasure(){
    RequestMeasureSi7021I2C2(Si7021_address_adj, Si7021_Measure_RH_No_Hold_Mode);
}

void GetHumidityData(){
    ReadHumidityFromSi7021I2C2(Si7021_address_adj,&tmpHumidity);
}

void GetTemperatureData(){
    Read16FromI2C2(Si7021_address_adj, Si7021_Read_T_from_Last_RH_Value, &tmpTemperature);
}

void UpdateTempAndHumidity(){
    Si7021_Temperature = ((175.72*(float)tmpTemperature)/65536.0) - 46.85;
    Si7021_Humidity = (((125.0)*(float)tmpHumidity)/65536.0) - 6;
}

// This function is meant to be called once per second.
void StepSi7021() {   
    if(isSi7021Configured==0) return;
    switch(currentState){
        case Measuring:
            GetHumidityData();            
            currentState=GettingTemperature;
            break;
        case GettingTemperature:
            GetTemperatureData();
            currentState=Calculation;
            break;
        case Idle:
            if(idleCounter++>=SECONDS_IN_IDLE){
                CallForTempHumidityMeasure();
                currentState = Measuring;
                idleCounter=0;
            }
            break;
        case Calculation:           
            UpdateTempAndHumidity();            
            currentState=Idle;
            break;       
        default:
            break;
    }
    
}