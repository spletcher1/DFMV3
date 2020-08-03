/* 
 * File:   Si7021.h
 * Author: scott
 *
 * Created on October 24, 2019, 5:58 PM
 */

#ifndef SI7021_H
#define	SI7021_H

unsigned char ConfigureSi7021();
void StepSi7021();
void UpdateHumidity(void);
void UpdateTemperature(void);
void RequestTempHumidityMeasure();
void GetTemperatureData();
void UpdateTempAndHumidity();


#endif	/* SI7021_H */

