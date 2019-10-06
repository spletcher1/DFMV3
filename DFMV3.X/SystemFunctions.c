#include "GlobalIncludes.h"

void Initialize_Basics(){
	// Disable the JTAG port
	DDPCONbits.JTAGEN = 0;
	// Make all analog input pins digital, by default.
	AD1PCFG = 0xFFFF;
}
void ConfigureInterrupts(void){
	INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
	INTEnableInterrupts();
}
void SetOptimizations(){
	// Don't use system config with the PMP.  It screws it up.
	SYSTEMConfig(GetSystemClock(), SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE); // Don't change my peripheral bus speed!
	//SYSTEMConfig(80000000, SYS_CFG_WAIT_STATES); // Don't change my peripheral bus speed!
	mOSCSetPBDIV( OSC_PB_DIV_2 );
	CheKseg0CacheOn();
	CHECONbits.PFMWS = 4;
}
void Startup(){
	Initialize_Basics();
	SetOptimizations();
	ConfigureInterrupts();
}
