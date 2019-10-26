#include <p32xxxx.h>
#include <plib.h>


#define BAUDRATE 400000 // Set at 400000 for normal EEPROM and 100000 for Propeller


extern long peripheral_speed;

// These parameters enable I2C in master mode.  Note that many settings specific to slave mode are
// ignored here, so another file should be set up to show the setup for slave mode.
// I2C is configured as enabled with receive on.
void I2C1_24LC256_Configure(void){
	int brg, baud;
	baud = BAUDRATE;
	brg = ((peripheral_speed/2)/baud)-2; // This sets the right brg assuming the baud rate shown above.

	OpenI2C1(I2C_ON | I2C_IDLE_CON | I2C_CLK_REL | I2C_STRICT_DIS | I2C_7BIT_ADD | I2C_SLW_EN |
			I2C_SM_DIS | I2C_GC_DIS | I2C_STR_DIS | I2C_ACK | I2C_ACK_EN | I2C_RCV_EN | I2C_STOP_DIS |
			I2C_RESTART_DIS | I2C_START_EN, brg);

}

// Return values
// 1 = Busy
// 0 = Not busy
unsigned char IsEEPROMBusy(unsigned char slaveaddress){
	unsigned char result;
	StartI2C1();
	IdleI2C1();
	MasterWriteI2C1(slaveaddress);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) result = 1;
	else result = 0;
	StopI2C1();IdleI2C1();
	return result;
}


// Return Codes:
// 0= No error
// 1 = Failed acknowledgment after write.
// 2 = Datasize too large.
char EEPROM_WriteChar(unsigned char slaveaddress, int dataaddress, char data, char waituntilwritecomplete) {

	// Begin the send sequence
	StartI2C1(); // Send the start bit.
	IdleI2C1(); // Wait until this is complete.

	MasterWriteI2C1(slaveaddress);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(dataaddress>>8);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 2; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(dataaddress & 0x00FF);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 3; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(data);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 4; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	I2CStop(I2C1);// Send the stop condition.
	IdleI2C1(); // Wait until done.
	
	if (waituntilwritecomplete) {
		// Now we need to wait until the write process is complete.
		// We can do this by repeatedly polling the slave device by
		// sending its address bit.  If it acknowledges, then it must
		// be done writing.  This usually takes about 5ms per byte.  
		// So we will poll every 50us.
		while(1) {
			Delay_50us(1);
			if(IsEEPROMBusy(slaveaddress)==0) break;
		}
	}
	return 0;

}

// Return Codes:
// 0= No error
// 1 = Failed acknowledgment after write.
// 2 = Memory address not a multiiple of 4 (possibility of writing across page boundary).
char EEPROM_WriteInt(unsigned char slaveaddress, int dataaddress, int data, char waituntilwritecomplete) {
	// Check to make sure that the memory address is divisible by 4.
	if ((dataaddress & 0x03)>0) return 2; // Not divisible by 4, so possiblity of writing over page boundary.

	// Begin the send sequence
	StartI2C1(); // Send the start bit.
	IdleI2C1(); // Wait until this is complete.

	MasterWriteI2C1(slaveaddress);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(dataaddress>>8);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(dataaddress & 0x00FF);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(data>>24);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	

	MasterWriteI2C1(data>>16);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	

	MasterWriteI2C1(data>>8);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	

	MasterWriteI2C1(data); // Only sends the lowest 8 bits because rest is truncated.
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	

	
	I2CStop(I2C1);// Send the stop condition.
	IdleI2C1(); // Wait until done.
	
	if (waituntilwritecomplete) {
		// Now we need to wait until the write process is complete.
		// We can do this by repeatedly polling the slave device by
		// sending its address bit.  If it acknowledges, then it must
		// be done writing.  This usually takes about 5ms per byte.  
		// So we will poll every 50us.
		while(1) {
			Delay_50us(1);
			if(IsEEPROMBusy(slaveaddress)==0) break;
		}
	}
	return 0;

}

// Return Codes:
// 0= No error
// 1 = Failed acknowledgment after write.
// 2 = Datasize too large.
char EEPROM_ReadCharDataBlock(unsigned char slaveaddress, int dataaddress, char *data, unsigned char numcharstoread) {
	char i2cdata[50];	
	char index;
	if(numcharstoread> 50) return 2;

	// Now begin the send sequence
	StartI2C1(); // Send the start bit.
	IdleI2C1(); // Wait until this is complete.

	// Send the slave address and memory start address	
	MasterWriteI2C1(slaveaddress);
	// For 24LC256, the top four bytes must be 1010 = 0xA0, then the three address bits.  The LSB is left open to indicate read/write.
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(dataaddress>>8);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(dataaddress & 0x00FF);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
		
	StopI2C1(); // Send the stop condition.
	IdleI2C1(); // Wait until done.
	
	RestartI2C1(); // Send restart.
	IdleI2C1();
	
	MasterWriteI2C1(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
	IdleI2C1();
	
	index=0;
	while(numcharstoread--){
		*(data++) = MasterReadI2C1();
		if(numcharstoread > 0) {AckI2C1();IdleI2C1();}
	}
	StopI2C1();
	IdleI2C1();
	return 0;
}

// Return Codes:
// 0= No error
// 1 = Failed acknowledgment after write.
char EEPROM_ReadChar(unsigned char slaveaddress, int dataaddress, char *data) {

	// Now begin the send sequence
	StartI2C1(); // Send the start bit.
	IdleI2C1(); // Wait until this is complete.

	// Send the slave address and memory start address	
	MasterWriteI2C1(slaveaddress);
	// For 24LC256, the top four bytes must be 1010 = 0xA0, then the three address bits.  The LSB is left open to indicate read/write.
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(dataaddress>>8);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 2; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(dataaddress & 0x00FF);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 3; // If this bit is 1, then slave failed to ackknowledge, so break.	
		
	StopI2C1(); // Send the stop condition.
	IdleI2C1(); // Wait until done.
	
	RestartI2C1(); // Send restart.
	IdleI2C1();
	
	MasterWriteI2C1(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
	IdleI2C1();
	
	*data = MasterReadI2C1();
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) {return 4;} // If this bit is 1, then slave failed to ackknowledge, so break.
	
	StopI2C1();
	IdleI2C1();
	return 0;
}

// Return Codes:
// 0= No error
// 1 = Failed acknowledgment after write.
char EEPROM_ReadInt(unsigned char slaveaddress, int dataaddress, int *data) {
	char tmp;
	int i;
	char s[25];
	// Now begin the send sequence
	StartI2C1(); // Send the start bit.
	IdleI2C1(); // Wait until this is complete.

	// Send the slave address and memory start address	
	MasterWriteI2C1(slaveaddress);
	// For 24LC256, the top four bytes must be 1010 = 0xA0, then the three address bits.  The LSB is left open to indicate read/write.
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(dataaddress>>8);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
	
	MasterWriteI2C1(dataaddress & 0x00FF);
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
		
	StopI2C1(); // Send the stop condition.
	IdleI2C1(); // Wait until done.
	
	RestartI2C1(); // Send restart.
	IdleI2C1();
	
	MasterWriteI2C1(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
	IdleI2C1();
	
	*data = 0;
	// Note that the masterread function waits until the receive bit is cleared.  Since all other relevant bits are 
	// also expected to be clear, there is no reason to check Idel before sending and acknowledgement.
	*data = MasterReadI2C1();
	AckI2C1();
	IdleI2C1();
	
	*data = (*data << 8) + MasterReadI2C1();
	AckI2C1();
	IdleI2C1();

	*data = (*data << 8) + MasterReadI2C1();
	AckI2C1();
	IdleI2C1();	
	
	*data = (*data << 8) + MasterReadI2C1();
	
	StopI2C1();
	IdleI2C1();

	return 0;
}


// Return Codes:
// 0= No error
// 1 = Failed acknowledgment after write.
// 2 = Datasize too large.
// DONT USE THIS FUNCTION UNTIL I UNDERSTAND PAGE BREAKS!!!!
char EEPROM_WriteCharDataBlock(unsigned char slaveaddress, int dataaddress, char *data, unsigned char datasize, char waituntilwritecomplete) {
	unsigned char i2cdata[64];	
	char index;
	unsigned int tmp;
	int i,totalbytestosend;
	unsigned char bytetosend;	
	i2cdata[0] = slaveaddress; // For 24LC256, the top four bytes must be 1010 = 0xA0, then the three address bits.  The LSB is left open to indicate read/write.
	i2cdata[1] = dataaddress >> 8; // High address byte.
	i2cdata[2] = dataaddress & 0x00FF;  // Low address byte.

	if(datasize > 64) return 2;

	// Now begin the send sequence
	StartI2C1(); // Send the start bit.
	IdleI2C1(); // Wait until this is complete.
	
	for(i = 0;i<datasize;i++) {
		i2cdata[i+3]=data[i];
	}

	index=0;
	totalbytestosend = datasize+3;
	while(totalbytestosend) {
		bytetosend = i2cdata[index++];
		tmp=MasterWriteI2C1(bytetosend);
		IdleI2C1();
		totalbytestosend--;		
		if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.
	}
	
	I2CStop(I2C1);// Send the stop condition.
	IdleI2C1(); // Wait until done.
	
	if (waituntilwritecomplete) {
		// Now we need to wait until the write process is complete.
		// We can do this by repeatedly polling the slave device by
		// sending its address bit.  If it acknowledges, then it must
		// be done writing.  This usually takes about 5ms per byte.  
		// So we will poll every 50us.
		while(1) {
			Delay_50us(3);
			StartI2C1();
			IdleI2C1();
			MasterWriteI2C1(i2cdata[0]);
			IdleI2C1();
			if(I2C1STATbits.ACKSTAT == 0)  {StopI2C1();IdleI2C1();return 0;} // The device has acknowledged, so stop and break.
			StopI2C1(); IdleI2C1();  // It did not acknowledge to stop and wait and start again.
		}
	}
	return 0;
}


