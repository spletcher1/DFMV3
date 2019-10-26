#ifndef __SP_I2C_PROP_H
#define __SP_I2C_PROP_H

void ConfigureI2C2(void);
unsigned char Read8FromI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned char *data);
unsigned char Read16FromI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data);
unsigned char Read32FromI2C2Backward(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data);
unsigned char Write8ToI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned char data);




#endif

