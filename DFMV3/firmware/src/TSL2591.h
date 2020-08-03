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
unsigned char CheckTimingAndGain();
void TimingAndGainChangeComplete();
void RequestFullLuminosity();
void RequestTimingAndGainCall();
void RequestTimingAndGainChangeCall();
void GetLux();

#endif	/* TSL2591_H */

