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

enum Si7021State {
    Measuring,
    Calculation,
    GettingTemperature,
    Si7021Idle,
};

#endif	/* SI7021_H */

