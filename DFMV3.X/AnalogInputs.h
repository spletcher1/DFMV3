/* 
 * File:   AnalogInputs.h
 * Author: Scott
 *
 * Created on June 10, 2013, 9:46 PM
 */

#ifndef ANALOGINPUTS_H
#define	ANALOGINPUTS_H

#ifdef	__cplusplus
extern "C" {
#endif


#define CONTINUOUS_SAMPLING 1
#define SAFEPUMPVALUE 300
//#define MANUAL_SAMPLING 1


void ConfigureAnalogInputs(void);
void StartContinuousSampling(void);
void StopSingleSampling(void);
int ReadADC(int input);
int ReadAvgADC(void);
int ReadAnalog1(void);
int ReadAnalog2(void);
void DisableADC(void);
void EnableADC(void);
void ClearAnalogValues(void);


#ifdef	__cplusplus
}
#endif

#endif	/* ANALOGINPUTS_H */

