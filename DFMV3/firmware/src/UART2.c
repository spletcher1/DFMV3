#include "GlobalIncludes.h"

#define MAXPACKETS 60  
#define STATUSREQUESTBYTE 0xFC
#define LATESTSTATUSREQUESTBYTE 0xEA
#define BUFFERRESETREQUESTBYTE 0xFE
#define SENDINSTRUCTIONBYTE 0xFD
#define SENDLINKAGEBYTE 0xFB
#define ACKBYTE 0xFA
#define COBSBUFFERSIZE (STATUSPACKETSIZE*(MAXPACKETS+2))+10
// This is here to avoid issues when other DFM send massive status
// packets that can be as big as COBSBUFFERSIZE
#define PACKETBUFFERSIZE COBSBUFFERSIZE
// This is set at PACKETBUFFERSIZE to avoid a crashing overflow
unsigned char cobsInstructionBuffer[PACKETBUFFERSIZE];
unsigned int cobsInstructionBufferLength;

//unsigned char cobsBuffer[COBSBUFFERSIZE];
// Adjust for DMA coherent memory as per : https://www.aidanmocke.com/blog/2019/01/08/DMA-Intro/
// Although this may only be required for PIC32MZ devices with an L1 Cache.
unsigned char __attribute__ ((coherent, aligned(8))) cobsBuffer[COBSBUFFERSIZE];

unsigned char preCodedBuffer[COBSBUFFERSIZE];
unsigned int cobsBufferLength;
extern int bufferSize;

char isAckReceived;
char waitingCounter=0;

enum UARTState volatile currentUARTState;

// Note that the packet buffer has to be big enough to
// properly catch status return calls from other DFM!!!
// Otherwise things will get out of whack and may lead
// to other DFM status returns being interpreted as 
// request calls.
unsigned char __attribute__ ((coherent, aligned(8))) packetBuffer[PACKETBUFFERSIZE];

extern unsigned char dfmID;
extern unsigned char isInDarkMode;
extern errorFlags_t volatile currentError;

extern struct StatusPacket emptyPacket;

char volatile waitingToDisable=-1;
char volatile waitingAfterEnable=-1;

static void UARTTxDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
         waitingToDisable=2;    
    }
    else if (event == DMAC_TRANSFER_EVENT_ERROR){
        currentError.bits.DMA_TX=1;
        waitingToDisable=2; 
    }
}


static void UARTRxDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        if(packetBuffer[1]==dfmID){
            currentUARTState = WaitingToProcess;           
        }
    }
    else if (event == DMAC_TRANSFER_EVENT_ERROR){
        currentError.bits.DMA_RX=1;        
    }
    
    // Keep listening.
    DMAC_ChannelTransfer(DMAC_CHANNEL_1, (const void *)(const void *)&U2RXREG, 1, 
                (const void *)packetBuffer, PACKETBUFFERSIZE, 1);
}

void ConfigureUART2(void) {
    // Note: As of now, the baud rate set for the parallax RFID reader is 2400.
    // Data bits = 8; no parity; stop bits = 1;

    //UART2_ReadCallbackRegister(UART2_ReadCallback,(uintptr_t)NULL);
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, UARTTxDmaChannelHandler, 0);
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_1, UARTRxDmaChannelHandler, 0);    
    currentUARTState = UARTIdle;
    isAckReceived=1;
    waitingCounter=0;    
  
    // Start Listening
    DMAC_ChannelTransfer(DMAC_CHANNEL_1, (const void *)(const void *)&U2RXREG, 1, 
                (const void *)packetBuffer, PACKETBUFFERSIZE, 1);           
}

void WriteCOBSBuffer(void){
    RX485_ENABLE_SEND();   
    waitingAfterEnable=2;
}

void EmptyPacketToUART2(){
    int i,counter=0;
    unsigned char *statusPointer = (unsigned char *)&emptyPacket.ID;
    
    for(i=0;i<STATUSPACKETSIZE;i++){
        preCodedBuffer[counter]=*(statusPointer+i);        
        counter++;        
    }    
    isAckReceived=1;
    if(counter>=COBSBUFFERSIZE)
        return; //Try to guard against overflow.    
    cobsBufferLength=encodeCOBS(preCodedBuffer,counter,cobsBuffer);
    if(cobsBufferLength<=0) return;
    cobsBuffer[cobsBufferLength++]=0x00;
    WriteCOBSBuffer();
}

void LatestStatusPacketToUART2(){
    struct StatusPacket *cs1;
    unsigned char *statusPointer;
    int i,counter=0;
    
    cs1 = GetLastestStatus();
    statusPointer= (unsigned char *)&cs1->ID;
        
    for(i=0;i<STATUSPACKETSIZE;i++){
        preCodedBuffer[counter]=*(statusPointer+i);        
        counter++;        
    }    
        
    
    // Setting this to 1 because we don't care whether it
    // was received or not, so assume it was.
    isAckReceived=1;
    if(counter>=COBSBUFFERSIZE)
        return; //Try to guard againt overflow.
    cobsBufferLength=encodeCOBS(preCodedBuffer,counter,cobsBuffer);
    if(cobsBufferLength<=0) return;
    cobsBuffer[cobsBufferLength++]=0x00;    
    WriteCOBSBuffer();
}

void CurrentStatusPacketSetToUART2(){
    struct StatusPacket *cs1;
    unsigned char *statusPointer;
    int numPacketsToSend;
    int i,j,counter=0;
    
    if(isAckReceived)
        SetTailPlaceHolder();
    else        
        ResetTail();
    
    if(bufferSize>MAXPACKETS)
        numPacketsToSend=MAXPACKETS;                        
    else
        numPacketsToSend = bufferSize;
    
    if(numPacketsToSend==0){
        EmptyPacketToUART2();
        return;
    }
       
    cs1 = GetNextStatusInLine();
    statusPointer= (unsigned char *)&cs1->ID;
        
    for(i=0;i<STATUSPACKETSIZE;i++){
        preCodedBuffer[counter++]=*(statusPointer+i);                
    }    
    
    for(j=1;j<numPacketsToSend;j++){
        cs1 = GetNextStatusInLine();
        statusPointer = (unsigned char *)&cs1->ID;
        for(i=0;i<STATUSPACKETSIZE;i++){
            preCodedBuffer[counter++]=*(statusPointer+i);                    
        }    
    }
   
    isAckReceived=0;
    
    if(counter>=COBSBUFFERSIZE)
        return; //Try to guard againt overflow.
    cobsBufferLength=encodeCOBS(preCodedBuffer,counter,cobsBuffer);
    if(cobsBufferLength<=0) return;
    cobsBuffer[cobsBufferLength++]=0x00;    
    WriteCOBSBuffer();    
}

unsigned char ValidateChecksum(int length){    
    int i;
    unsigned int checksum=0,actual;
    for(i=0;i<(length-4);i++)
        checksum+=cobsInstructionBuffer[i];
    checksum = (checksum ^ 0xFFFFFFFF)+1;
    
    actual=(unsigned int)(cobsInstructionBuffer[(length-4)]<<24);
    actual+=(unsigned int)(cobsInstructionBuffer[(length-3)]<<16);
    actual+=(unsigned int)(cobsInstructionBuffer[(length-2)]<<8);
    actual+=(unsigned int)(cobsInstructionBuffer[(length-1)]);
    // Checksum is calculated excluding the header characters.    
    return (checksum==actual);
}

void SendAck(){
    // Need tos end 0x00 char to terminate packets on all listening
    // DFM.    
    cobsBuffer[0]=2;  
    cobsBuffer[1]=dfmID;  
    cobsBuffer[2]=0x00;
    cobsBufferLength=3;
    WriteCOBSBuffer();
}

void SendNAck(){    
    cobsBuffer[0]=2;  
    cobsBuffer[1]=0xFE;  
    cobsBuffer[2]=0x00;
    cobsBufferLength=3;
    WriteCOBSBuffer();  
}

void ExecuteInstructionPacket(){    
    unsigned char index=2,i;
    unsigned int freq,pw,decay,delay,maxTime;
    int thresh[12];
    if(cobsInstructionBuffer[index++]==0)
        SetDarkMode(0);
    else
        SetDarkMode(1);
    
    freq = (unsigned int)(cobsInstructionBuffer[index]<<8)+(unsigned int)(cobsInstructionBuffer[index+1]);
    index+=2;
    pw = (unsigned int)(cobsInstructionBuffer[index]<<8)+(unsigned int)(cobsInstructionBuffer[index+1]);
    index+=2;
    decay = (unsigned int)(cobsInstructionBuffer[index]<<8)+(unsigned int)(cobsInstructionBuffer[index+1]);
    index+=2;
    delay = (unsigned int)(cobsInstructionBuffer[index]<<8)+(unsigned int)(cobsInstructionBuffer[index+1]);
    index+=2;
    maxTime = (unsigned int)(cobsInstructionBuffer[index]<<8)+(unsigned int)(cobsInstructionBuffer[index+1]);
    index+=2;   
    for(i=0;i<12;i++){
        thresh[i]=(unsigned int)(cobsInstructionBuffer[index]<<8)+(unsigned int)(cobsInstructionBuffer[index+1]);
        index+=2;
    }
    SetOptoParameters(freq,pw);
    SetLEDParams(decay,delay,maxTime);
    SetLEDThresholds(thresh);                   
}
void ExecuteLinkagePacket(){    
    unsigned char index=2,i;   
    unsigned char linkage[12];
    for(i=0;i<12;i++){
        linkage[i]=(cobsInstructionBuffer[index+i]);        
    }
    SetLEDLinkFlags(linkage);
}

void ProcessPacket() {    
    unsigned int packetSize;
    // With COBS encoding, packetBuffer[0] is the COBS encoder.]
    // because the first two byte of the actual packet will never be zero
    // we can query these before decoding to determine whether we have to do 
    // that or not.
    // Note that only the instruction packet needs to be decoded, the others will
    // not have any zero bytes and so can be read directly, once the header COBS encoder
    // is accounted for.
    if(packetBuffer[1]!=dfmID) {
        // Should never be here with new packet handling code    
        currentError.bits.PACKET=1;
        return; // Packet not for me    
    }
    if (isInDarkMode == 0 && packetBuffer[2]!=ACKBYTE) FLIP_GREEN_LED();
    if(packetBuffer[2]==STATUSREQUESTBYTE){
        if(packetBuffer[1]==dfmID && packetBuffer[3]==dfmID){
            waitingCounter=15;
            currentUARTState = WaitingToSendStatus; 
        }
    }
    else if(packetBuffer[2]==LATESTSTATUSREQUESTBYTE){
        if(packetBuffer[1]==dfmID && packetBuffer[3]==dfmID){
            waitingCounter=15;
            currentUARTState = WaitingToSendLatestStatus; 
        }
    }
    else if(packetBuffer[2]==BUFFERRESETREQUESTBYTE){        
        if(packetBuffer[1]==dfmID && packetBuffer[3]==dfmID){            
            InitializeStatusPacketBuffer();
            isAckReceived=1;
            waitingCounter=15;
            currentUARTState = WaitingToAck;       
            return;
        }
    }
    else if(packetBuffer[2]==SENDINSTRUCTIONBYTE){
        packetSize=0;
        while(packetBuffer[packetSize]!=0){
            packetSize++;;
        };        
        cobsInstructionBufferLength=decodeCOBS(packetBuffer,packetSize,cobsInstructionBuffer);        
        if(!ValidateChecksum(41) || (cobsInstructionBufferLength<=0)) {
            waitingCounter=15;
            currentUARTState = WaitingToNAck; 
            return;
        }
        waitingCounter=15;
        currentUARTState = WaitingToAck;  
        ExecuteInstructionPacket();
    }   
    else if(packetBuffer[2]==SENDLINKAGEBYTE){
        packetSize=0;
        while(packetBuffer[packetSize]!=0){
            packetSize++;;
        };        
        cobsInstructionBufferLength=decodeCOBS(packetBuffer,packetSize,cobsInstructionBuffer);        
        if(!ValidateChecksum(18) || (cobsInstructionBufferLength<=0)) {
            waitingCounter=15;
            currentUARTState = WaitingToNAck; 
            return;
        }
        waitingCounter=15;
        currentUARTState = WaitingToAck;  
        ExecuteLinkagePacket();
    }
     else if(packetBuffer[2]==ACKBYTE){    
        if(packetBuffer[1]==dfmID && packetBuffer[3]==dfmID){
            isAckReceived=1;         
        }
        currentUARTState=ClearPacket;
    }
     else {
         // Wonder how many times we get here, suggesting weird packet
        currentError.bits.PACKET=1;
    }
}

void StepUART(){       
    switch(currentUARTState){
        case UARTIdle:
            break;
        case WaitingToProcess:            
            ProcessPacket();       
        case WaitingToAck:            
            if(waitingCounter>=0){
                if(waitingCounter--<=0){
                    SendAck();                
                    currentUARTState=ClearPacket;
                    waitingCounter=-1;
                }
            }
            break;
        case WaitingToNAck:
            if(waitingCounter>=0){
                if(waitingCounter--<=0){
                    SendNAck();                
                    currentUARTState=ClearPacket;
                    waitingCounter=-1;
                }
            }
            break;
        case WaitingToSendStatus:            
            if(waitingCounter>=0){
                if(waitingCounter--<=0){
                    CurrentStatusPacketSetToUART2();  
                    currentUARTState=ClearPacket;
                    waitingCounter=-1;
                }
            }
            break;
        case WaitingToSendLatestStatus:            
            if(waitingCounter>=0){
                if(waitingCounter--<=0){
                    LatestStatusPacketToUART2();  
                    currentUARTState=ClearPacket;
                    waitingCounter=-1;
                }
            }
            break;
        case ClearPacket:                      
            currentUARTState=UARTIdle;
            break;            
    }
    if(waitingToDisable>=0){
        if(waitingToDisable--<=0) {
            RX485_DISABLE_SEND();  
            waitingToDisable=-1;
        }
    }
    if(waitingAfterEnable>=0){
        if(waitingAfterEnable--<=0) {            
            DMAC_ChannelTransfer(DMAC_CHANNEL_0, (const void *)cobsBuffer, cobsBufferLength, 
                (const void *)&U2TXREG, 1, 1);            
            waitingAfterEnable=-1;
        }
    }
    
    
    
}
unsigned int encodeCOBS(unsigned char* buffer,unsigned int bytesToEncode, unsigned char* encodedBuffer)
    {
        unsigned int read_index  = 0;
        unsigned write_index = 1;
        unsigned int code_index  = 0;
        unsigned char coded = 1;

        while (read_index < bytesToEncode)
        {

            if (buffer[read_index] == 0)
            {
                encodedBuffer[code_index] = coded;
                coded = 1;
                code_index = write_index++;
                read_index++;
            }
            else
            {
                encodedBuffer[write_index++] = buffer[read_index++];
                coded++;
                if (coded == 0xFF)
                {
                    encodedBuffer[code_index] = coded;
                    coded = 1;
                    code_index = write_index++;
                }
            }
        }
        encodedBuffer[code_index] = coded;
        return write_index;
    }

unsigned int decodeCOBS(volatile unsigned char* encodedBuffer,unsigned int bytesToEncode, unsigned char* decodedBuffer)
    {
        unsigned int read_index  = 0;
        unsigned int write_index = 0;
        unsigned char coded = 0;
        unsigned char i =  0;

        if (bytesToEncode == 0)
            return 0;

        while (read_index < bytesToEncode)
        {
            coded = encodedBuffer[read_index];
            if (read_index + coded > bytesToEncode && coded != 1)
            {
                return 0;
            }
            read_index++;
            for (i = 1; i < coded; i++)
            {
                decodedBuffer[write_index++] = encodedBuffer[read_index++];
            }
            if (coded != 0xFF && read_index != bytesToEncode)
            {
                decodedBuffer[write_index++] = '\0';
            }
        }
        return write_index;
    }

