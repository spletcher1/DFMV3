#ifndef LEDCONTROL_H
#define	LEDCONTROL_H

void StepLEDControl();
void InitializeLEDControl(unsigned int decayval,unsigned int delayval,unsigned int maxtimeval);
void SetDelay(unsigned int delay);
void SetDecay(unsigned int decay);
void SetMaxTimeOn(unsigned int maxTime);
void SetLEDThresholds(int *thresh);
void ClearLEDThresholds();
void SetLEDLinkFlags(unsigned char *linkdefs);

typedef union {
    unsigned short ledField;
    struct LEDBits {
        unsigned LED1 : 1;
        unsigned LED2 : 1;
        unsigned LED3 : 1;
        unsigned LED4 : 1;
        unsigned LED5 : 1;
        unsigned LED6 : 1;
        unsigned LED7 : 1;
        unsigned LED8 : 1;
        unsigned LED9 : 1;
        unsigned LED10 : 1;
        unsigned LED11 : 1;
        unsigned LED12 : 1;
        unsigned TBD1 : 1;
        unsigned TBD2 : 1;
        unsigned TBD3 : 1;
        unsigned TBD4 : 1;
    } bits;
} LEDFLAGS;




#endif	/* LEDCONTROL_H */

