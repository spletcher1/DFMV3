
#ifndef __SP_UART1_H
#define __SP_UART1_H


void ConfigureUART2(void);
void ProcessPacket();
void StringToUART2(const char* buffer);
void CharToUART2(char c);
void inline DisableUARTInterrupts();
void inline EnableUARTInterrupts();
unsigned int encodeCOBS(unsigned char* buffer,unsigned int bytesToEncode, unsigned char* encodedBuffer); 
unsigned int decodeCOBS(unsigned char* encodedBuffer,unsigned int bytesToEncode, unsigned char* decodedBuffer);

#endif
