#ifndef LEDCONTROL_H
#define	LEDCONTROL_H

void StepLEDControl();
void InitializeLEDControl(unsigned int decayval,unsigned int delayval,unsigned int maxtimeval);
void SetDelay(unsigned int delay);
void SetDecay(unsigned int decay);
void SetMaxTimeOn(unsigned int maxTime);
void SetLEDThresholds(int *thresh);
void ClearLEDThresholds();

#endif	/* LEDCONTROL_H */

