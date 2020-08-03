#include "GlobalIncludes.h"

void Delay10us(int dwCount)
{
	volatile int _dcnt;

	_dcnt = dwCount*((int)(0.00001/(1.0/80000000ul)/10));
	while(_dcnt--)
	{
		#if defined(__C32__)
			Nop();
			Nop();
			Nop();
		#endif
	}
}

void DelayMs(int ms)
{
    unsigned char i;
    while(ms--)
    {
        i=4;
        while(i--)
        {
            Delay10us(25);
        }
    }
}

