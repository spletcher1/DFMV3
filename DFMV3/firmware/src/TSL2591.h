/* 
 * File:   TSL2591.h
 * Author: scott
 *
 * Created on October 24, 2019, 5:59 PM
 */

#ifndef TSL2591_H
#define	TSL2591_H

unsigned char ConfigureTSL2591();
void StepTSL2591();
void StoreFullLuminosity();
void CheckTimingAndGain();
void TimingAndGainChangeComplete();

enum TSL2591State {
    RequestLuminosity,    
    LuxCalculation,
    RequestTimingAndGain,
    RequestTimingAndGainChange,
    LuxReady,
    Idle,
};

#endif	/* TSL2591_H */

