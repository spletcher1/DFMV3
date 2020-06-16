#include "GlobalIncludes.h"

#define Si7021_address                          0x40

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


#define SECONDS_IN_IDLE 5

unsigned int Si7021_Humidity;
unsigned int Si7021_Temperature;
unsigned int tmpHumidity, tmpTemperature;

extern errorFlags_t volatile currentError;

extern unsigned char i2cData[4];   


unsigned char IsSi7021Ready() {
    unsigned char found;
    unsigned char data[1];
    data[0]=Si7021_Read_Config_Register;
    I2C2_WriteRead(Si7021_address,&data[0],1,&found,1); 
    while(I2C2_IsBusy());
    if (found != 0x00)
        return 1;
    else
        return 0;
}

unsigned char ConfigureSi7021() {   
    Si7021_Humidity = Si7021_Temperature = tmpHumidity = tmpTemperature = 0;
    DelayMs(2);
    if (!IsSi7021Ready()) {
        return 0;
    }    
    return 1;
}

void RequestTempHumidityMeasure() {
    unsigned char reg = Si7021_Measure_RH_Hold_Mode; //Si7021_Measure_RH_No_Hold_Mode;    
    I2C2_WriteRead(Si7021_address,&reg,1,&i2cData[0],2);      
    Delay10us(4);
    
    //while(I2C2_IsBusy());
    //reg=data[0];
    //I2C2_Write(Si7021_address,&data[0],2);         
}

void UpdateHumidity(){    
    tmpHumidity = (i2cData[0]<<8)+i2cData[1];      
}

// Not used with Hold mode request
void GetHumidityData() {                 
    I2C2_Read(Si7021_address,&i2cData[0],2);      
}

void UpdateTemperature(){
    tmpTemperature = (i2cData[0]<<8)+i2cData[1];   
}
void GetTemperatureData() {
    unsigned char reg = Si7021_Read_T_from_Last_RH_Value;        
    I2C2_WriteRead(Si7021_address,&reg,1,&i2cData[0],2); 
    Delay10us(4);
}

void UpdateTempAndHumidity() {
    float t, h;
    t = ((175.72 * (float) tmpTemperature) / 65536.0) - 46.85;
    h = (((125.0)*(float) tmpHumidity) / 65536.0) - 6;
    // Temperature and Humidity will be encoded as integers to three decimal places.
    Si7021_Temperature = (unsigned int) (t * 1000);
    Si7021_Humidity = (unsigned int) (h * 1000);
}

// This function is meant to be called once per second.
/*
void StepSi7021() {
    if (isSi7021Configured == 0) return;
    switch (currentState_Si) {
        case RequestMeasure:
            RequestTempHumidityMeasure();            
            break;      
        case GetTemperature:
            GetTemperatureData();        
            break;
        case Si7021Idle:
            if (idleCounter_Si++ >= SECONDS_IN_IDLE) {                
                currentState_Si = RequestMeasure;
                idleCounter_Si = 0;
            }
            break;
        case Calculate:
            if(currentError.bits.I2C==1 || currentError.bits.Si7021==1){
                Si7021_Temperature = 0;
                Si7021_Humidity =0;
            }
            else
                UpdateTempAndHumidity();
            currentState_Si = Si7021Idle;
            break;
        default:
            break;
    }

}
 * */