
#ifndef ANALOGINPUTS_H
#define	ANALOGINPUTS_H

#include "PacketManager.h"

void ConfigureAnalogInputs(void);
void StartContinuousSampling(void);
void FillCurrentStatus(struct StatusPacket *cS);
void StepADC();
#endif	/* ANALOGINPUTS_H */

