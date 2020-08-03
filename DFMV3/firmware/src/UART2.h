/* 
 * File:   UART2.h
 * Author: scott
 *
 * Created on June 6, 2020, 10:41 AM
 */

#ifndef UART2_H
#define	UART2_H

#ifdef	__cplusplus
extern "C" {
#endif

void ConfigureUART2(void);
void ProcessPacket();
unsigned int encodeCOBS(unsigned char* buffer,unsigned int bytesToEncode, unsigned char* encodedBuffer); 
unsigned int decodeCOBS(volatile unsigned char* encodedBuffer,unsigned int bytesToEncode, unsigned char* decodedBuffer);
void WriteCOBSBuffer(void);
void SendAck(void);
void SendNAck(void);
void StepUART(void);

enum PacketState {
    None,
    GettingID,
    InPacket,
    Ignoring,
};

enum UARTState {
    UARTIdle,   
    WaitingToProcess,
    ClearPacket,
    WaitingToAck,
    WaitingToNAck,
    WaitingToSendStatus,
    WaitingToSendLatestStatus     
};



#ifdef	__cplusplus
}
#endif

#endif	/* UART2_H */

