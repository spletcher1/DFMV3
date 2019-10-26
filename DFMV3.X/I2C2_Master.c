#include "GlobalIncludes.h"

// Normally I2C devices run at either 100khz or 400 khz, although some can do 2MHz
// The propellor seems to only manage 100khz
#define BAUDRATE 400000 // Set at 400000 for normal EEPROM and 100000 for Propeller
int ErrorCount;

void ConfigureI2C2(void) {
    int brg;
    if ((GetPeripheralClock()) == 40000000) {
        if (BAUDRATE == 100000)
            brg = 0x0C6;
        if (BAUDRATE == 400000)
            brg = 0x030;
    }
    SetPriorityIntI2C2(5);
    EnableIntMI2C2;
    EnableIntBI2C2;
    INTClearFlag(INT_I2C2M);
    INTClearFlag(INT_I2C2B);
    OpenI2C2(I2C_ON | I2C_IDLE_CON | I2C_CLK_REL | I2C_STRICT_DIS | I2C_7BIT_ADD | I2C_SLW_EN |
			I2C_SM_DIS | I2C_GC_DIS | I2C_STR_DIS | I2C_ACK | I2C_ACK_EN | I2C_RCV_EN | I2C_STOP_DIS |
			I2C_RESTART_DIS | I2C_START_EN, brg);
    //OpenI2C2(I2C_EN, brg);    
}
void __ISR(_I2C_2_VECTOR, IPL5AUTO) I2C2InterruptServiceRoutine(void) {
    // check for MASTER and Bus events and respond accordingly
    if (IFS1bits.I2C2SIF == 1) { // Should never be here because we don't operate as slave.
        //if(IsInDarkMode==0)
        //    IO_LED6_ON();
        mI2C2MClearIntFlag();
        return;
    }
    if (IFS1bits.I2C2BIF == 1) {
        //if(IsInDarkMode==0)
        //    IO_LED5_ON();
        mI2C2BClearIntFlag();
        return;
    }
    INTClearFlag(INT_I2C2M);      
}

// This function is simplified for single byte addresses
// slaveaddress should already be shifted left to most significant bits to leave
// LSB open for read/write indication.
unsigned char Read8FromI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned char *data) {

	// Now begin the send sequence
	StartI2C2(); // Send the start bit.
	IdleI2C2(); // Wait until this is complete.

	// Send the slave address and memory start address	
	MasterWriteI2C2(slaveaddress);	
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C2(dataaddress);
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 2; // If this bit is 1, then slave failed to ackknowledge, so break.	
		
	StopI2C2(); // Send the stop condition.
	IdleI2C2(); // Wait until done.
	
	RestartI2C2(); // Send restart.
	IdleI2C2();
	
	MasterWriteI2C2(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
	IdleI2C2();
	
	*data = MasterReadI2C2();
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) {return 4;} // If this bit is 1, then slave failed to ackknowledge, so break.
	
	StopI2C2();
	IdleI2C2();
	return 0;
}

unsigned char Read16FromI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data) {

	// Now begin the send sequence
	StartI2C2(); // Send the start bit.
	IdleI2C2(); // Wait until this is complete.

	// Send the slave address and memory start address	
	MasterWriteI2C2(slaveaddress);	
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C2(dataaddress);
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 2; // If this bit is 1, then slave failed to ackknowledge, so break.	
		
	StopI2C2(); // Send the stop condition.
	IdleI2C2(); // Wait until done.
	
	RestartI2C2(); // Send restart.
	IdleI2C2();
	
	MasterWriteI2C2(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
	IdleI2C2();
	
	*data = MasterReadI2C2();
	AckI2C2();
	IdleI2C2();
    
    *data = (*data << 8) + MasterReadI2C2();
	AckI2C2();
	IdleI2C2();
	
	StopI2C2();
	IdleI2C2();
	return 0;
}

unsigned char Read32FromI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data) {

	// Now begin the send sequence
	StartI2C2(); // Send the start bit.
	IdleI2C2(); // Wait until this is complete.

	// Send the slave address and memory start address	
	MasterWriteI2C2(slaveaddress);	
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C2(dataaddress);
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 2; // If this bit is 1, then slave failed to ackknowledge, so break.	
		
	StopI2C2(); // Send the stop condition.
	IdleI2C2(); // Wait until done.
	
	RestartI2C2(); // Send restart.
	IdleI2C2();
	
	MasterWriteI2C2(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
	IdleI2C2();
	
	*data = MasterReadI2C2();
	AckI2C2();
	IdleI2C2();
    
    *data = (*data << 8) + MasterReadI2C2();
	AckI2C2();
	IdleI2C2();
	
    *data = (*data << 8) + MasterReadI2C2();
	AckI2C2();
	IdleI2C2();
    
    *data = (*data << 8) + MasterReadI2C2();
	AckI2C2();
	IdleI2C2();
    
	StopI2C2();
	IdleI2C2();
	return 0;
}

unsigned char Read32FromI2C2Backward(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data) {
    unsigned char a,b,c,d;
	// Now begin the send sequence
	StartI2C2(); // Send the start bit.
	IdleI2C2(); // Wait until this is complete.

	// Send the slave address and memory start address	
	MasterWriteI2C2(slaveaddress);	
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C2(dataaddress);
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 2; // If this bit is 1, then slave failed to ackknowledge, so break.	
		
	StopI2C2(); // Send the stop condition.
	IdleI2C2(); // Wait until done.
	
	RestartI2C2(); // Send restart.
	IdleI2C2();
	
	MasterWriteI2C2(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
	IdleI2C2();
	
	*data = 0;
	// Note that the masterread function waits until the receive bit is cleared.  Since all other relevant bits are 
	// also expected to be clear, there is no reason to check Idel before sending and acknowledgement.
	a = MasterReadI2C2();
	AckI2C2();
	IdleI2C2();
	
	b = MasterReadI2C2();
	AckI2C2();
	IdleI2C2();

	c = MasterReadI2C2();
	AckI2C2();
	IdleI2C2();	
	
	d = MasterReadI2C2();
	
	StopI2C2();
	IdleI2C2();
    
     *(data)=(d<<24) + (c<<16) + (b<<8) + a;
	return 0;
}


// This function is simplified for single byte addresses
unsigned char Write8ToI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned char data) {

	// Begin the send sequence
	StartI2C2(); // Send the start bit.
	IdleI2C2(); // Wait until this is complete.

	MasterWriteI2C2(slaveaddress);
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
		
	MasterWriteI2C2(dataaddress);
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 2; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C2(data);
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 4; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	I2CStop(I2C2);// Send the stop condition.
	IdleI2C2(); // Wait until done.
		
	return 0;

}
