/* 
 * File:   I2C2.h
 * Author: scott
 *
 * Created on June 6, 2020, 8:50 PM
 */

#ifndef I2C2_H
#define	I2C2_H

#ifdef	__cplusplus
extern "C" {
#endif

void ConfigureI2C2(void);
void StepI2C(void);
enum I2CState {
    RequestMeasure,    
    Calculate,
    GetTemperature,   
    RequestLuminosity,    
    LuxCalculation,
    RequestTimingAndGain,
    RequestTimingAndGainChange,
    LuxReady,
    Idle,
};

#ifdef	__cplusplus
}
#endif

#endif	/* I2C2_H */

