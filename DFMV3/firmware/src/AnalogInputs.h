/* 
 * File:   AnalogInputs.h
 * Author: scott
 *
 * Created on June 6, 2020, 12:45 PM
 */

#ifndef ANALOGINPUTS_H
#define	ANALOGINPUTS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "PacketManager.h"

    
void ConfigureAnalogInputs(void);
void FillCurrentStatus(struct StatusPacket *cS);
void StepADC();


#ifdef	__cplusplus
}
#endif

#endif	/* ANALOGINPUTS_H */

