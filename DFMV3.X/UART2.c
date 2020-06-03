#include "GlobalIncludes.h"

//Actually in PIC32MX795F512L uC the UARTs numbering is like this one:

//UART1A = UART1
//UART2A = UART3
//UART3A = UART2
//UART1B = UART4
//UART2B = UART6
//UART3B = UART5



#define MAXPACKETS 60  
#define STATUSREQUESTBYTE 0xFC
#define BUFFERRESETREQUESTBYTE 0xFE
#define SENDINSTRUCTIONBYTE 0xFD
#define SENDLINKAGEBYTE 0xFB
#define ACKBYTE 0xFA
#define PACKETBUFFERSIZE 500
#define COBSBUFFERSIZE (STATUSPACKETSIZE*(MAXPACKETS+2))+10

unsigned char cobsInstructionBuffer[100];
unsigned int cobsInstructionBufferLength;

unsigned char cobsBuffer[COBSBUFFERSIZE];
unsigned char preCodedBuffer[COBSBUFFERSIZE];
unsigned int cobsBufferLength;
extern int bufferSize;

char isAckReceived;

enum PacketState volatile currentPacketState;

// Note that the packet buffer has to be big enough to
// properly catch status return calls from other DFM!!!
// Otherwise things will get out of whack and may lead
// to other DFM status returns being interpreted as 
// request calls.
unsigned char volatile packetBuffer[PACKETBUFFERSIZE];
unsigned int volatile packetIndex;
unsigned int volatile lastPacketSize;

extern unsigned char dfmID;
extern unsigned char isInDarkMode;
extern errorFlags_t volatile currentError;

extern struct StatusPacket emptyPacket;

/////////////////////////////////////////////////////////////////
// Select one to specify configuration.
// Baud Rate 19200
// Baud rate 38400
// Baud rate 115200
// #define TARGET_BAUD_RATE  921600
//#define TARGET_BAUD_RATE  115200
#define TARGET_BAUD_RATE  250000

/////////////////////////////////////////////////////////////////
void inline DisableUARTInterrupts(){
    INTEnable(INT_U2RX,INT_DISABLED);
	INTEnable(INT_U2E,INT_DISABLED);    
}

void inline EnableUARTInterrupts(){
    INTEnable(INT_U2RX,INT_ENABLED);
	INTEnable(INT_U2E,INT_ENABLED);  
    INTClearFlag(INT_U2RX);
    INTClearFlag(INT_U2TX); 
}


void CharToUART2(char c){
    RX485_ENABLE_SEND();
    while (!UARTTransmitterIsReady(UART2));
        UARTSendDataByte(UART2, c);    
    RX485_DISABLE_SEND();
}

void ConfigureUART2Interrupts(){
	// For now we interrupt on RX
	INTSetVectorPriority(INT_UART_2_VECTOR,INT_PRIORITY_LEVEL_6);
	INTClearFlag(INT_U2RX);
    INTClearFlag(INT_U2TX);    
	INTEnable(INT_U2RX,INT_ENABLED);
	INTEnable(INT_U2E,INT_ENABLED);
}

void ConfigureUART2(void) {
    // Note: As of now, the baud rate set for the parallax RFID reader is 2400.
    // Data bits = 8; no parity; stop bits = 1;

    //UARTConfigure(UART2, UART_ENABLE_HIGH_SPEED);
    UARTConfigure(UART2,0);
    UARTSetFifoMode(UART2, UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART2, GetPeripheralClock(), TARGET_BAUD_RATE);
    UARTEnable(UART2,UART_ENABLE_FLAGS(UART_ENABLE | UART_TX | UART_RX | UART_PERIPHERAL));

    ConfigureUART2Interrupts();
    packetIndex=0;
    currentPacketState = None;
    isAckReceived=0;
       
}

// Interestingly, this interrupt seems to be called 9 times to process
// a 9 Byte status packet!  The duration of each interrupt call is about 1us, with
// O3 optimization.

// Additional timings revealed that it takes about 570us to receive the status request
// and then complete sending the response package. This seems well within the needed
// interval. This timing could easily handle even 100 DFM.
void __ISR(_UART2_VECTOR, IPL6SOFT) UART2Interrupt(void){
	int error;
	unsigned char data,tmp;
	error = UART2GetErrors();	
	if (error > 0) {
		if (U2STA & 0x02) { //Overflow Error
            while(DataRdyUART2())            
                data = UARTGetDataByte(UART2);   
			U2STAbits.OERR = 0;
			INTClearFlag(INT_U2E);			
            INTClearFlag(INT_U2RX);	           
            currentError.bits.OERR=1;
            packetIndex=0;
            currentPacketState=None;
            return;
		}
		else if(U2STA & 0x04) {
            while(DataRdyUART2())            
                data = UARTGetDataByte(UART2);   
			U2STAbits.FERR=0;				
			INTClearFlag(INT_U2E);
            currentError.bits.FERR=1;
            INTClearFlag(INT_U2RX);	
            packetIndex=0;		
            currentPacketState=None;
            return;
		}	
        else if(U2STA & 0x08) {
            currentError.bits.PERR=1;
            while(DataRdyUART2())            
                data = UARTGetDataByte(UART2);            
            UART2ClearAllErrors();            
            INTClearFlag(INT_U2E);
            INTClearFlag(INT_U2RX);	
            packetIndex=0;
            currentPacketState=None;
        }
	}	
    while(DataRdyUART2()) {
		data=UARTGetDataByte(UART2);  
        switch(currentPacketState){
            case None:
                packetBuffer[packetIndex++]=data;     
                currentPacketState = GettingID;
                break;
            case GettingID:
                if(data==dfmID){
                    packetBuffer[packetIndex++]=data;  
                    currentPacketState =InPacket;
                }
                else if(data==0x00){ // will be here when other DFMs ack
                    currentPacketState = None;
                    packetIndex=0;
                }
                else {
                    currentPacketState = Ignoring;
                }
                break;
            case InPacket:               
                if(data==0x00){                   
                    currentPacketState = Complete;
                    lastPacketSize=packetIndex;
                    packetIndex=0;
                }
                else {
                   packetBuffer[packetIndex++]=data;                 
                }
                break;           
            case Ignoring:
                if(data==0x00){
                    currentPacketState = None;
                    packetIndex=0;
                }
                break; 
            case Complete:     
                // This needs to ignore all data until process packet 
                // allows it to collect again.  We really shouldn't
                // even acknowledge a termination 0 to send bacy to None because
                // in theory we could still be processing the packetBuffer.
                // So we don't do anything. Just ignore the data.          
                break;
        }
        if(packetIndex>=PACKETBUFFERSIZE){            
            currentError.bits.PACKET=1;   
            currentPacketState=None;
            packetIndex=0;
        }
    }    
    INTClearFlag(INT_U2RX);		
}

void CurrentStatusPacketSetToUART2(int numPacketsToSend){
    struct StatusPacket *cs1;
    unsigned char *statusPointer;
    
    if(isAckReceived)
        SetTailPlaceHolder();
    else
        ResetTail();
    
    
    cs1 = GetNextStatusInLine();
    statusPointer= (char *)&cs1->ID;
    int i,j,counter=0;
    
    for(i=0;i<STATUSPACKETSIZE;i++){
        preCodedBuffer[counter]=*(statusPointer+i);        
        counter++;        
    }    
    
    for(j=1;j<numPacketsToSend;j++){
        cs1 = GetNextStatusInLine();
        statusPointer = (char *)&cs1->ID;
        for(i=0;i<STATUSPACKETSIZE;i++){
            preCodedBuffer[counter]=*(statusPointer+i);        
            counter++;    
        }    
    }
   
    cobsBufferLength=encodeCOBS(preCodedBuffer,counter,cobsBuffer);
    cobsBuffer[cobsBufferLength++]=0x00;
    RX485_ENABLE_SEND();
    for(i=0;i<cobsBufferLength;i++){
        while (!UARTTransmitterIsReady(UART2));
        UARTSendDataByte(UART2, *(cobsBuffer+i));       
    }    
    RX485_DISABLE_SEND();
    isAckReceived=0;
}

void EmptyPacketToUART2(){
    int i,counter=0;
    unsigned char *statusPointer = (char *)&emptyPacket.ID;
    
    for(i=0;i<STATUSPACKETSIZE;i++){
        preCodedBuffer[counter]=*(statusPointer+i);        
        counter++;        
    }    
    
    cobsBufferLength=encodeCOBS(preCodedBuffer,counter,cobsBuffer);
    cobsBuffer[cobsBufferLength++]=0x00;
    RX485_ENABLE_SEND();
    for(i=0;i<cobsBufferLength;i++){
        while (!UARTTransmitterIsReady(UART2));
        UARTSendDataByte(UART2, *(cobsBuffer+i));       
    }    
    RX485_DISABLE_SEND();
}

void SendAck(){
    // Need tos end 0x00 char to terminate packets on all listening
    // DFM.
    CharToUART2(dfmID);
    CharToUART2(0x00); 
}

void SendNAck(){
    CharToUART2(0xFE);
    CharToUART2(0x00);
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
    int num;
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
            DelayMs(15);
            if(bufferSize>MAXPACKETS) {
                num=MAXPACKETS;                 
            }
            else{
                num = bufferSize;
            }
            if(num>0)
                CurrentStatusPacketSetToUART2(num);   
            else
                EmptyPacketToUART2();
        }
    }
    else if(packetBuffer[2]==BUFFERRESETREQUESTBYTE){
        if(packetBuffer[1]==dfmID && packetBuffer[3]==dfmID){
            InitializeStatusPacketBuffer();
            DelayMs(15);
            SendAck();
            return;
        }
    }
    else if(packetBuffer[2]==SENDINSTRUCTIONBYTE){
        cobsInstructionBufferLength=decodeCOBS(packetBuffer,lastPacketSize,cobsInstructionBuffer);        
        if(!ValidateChecksum(41)) {
            DelayMs(15);
            SendNAck();
            return;
        }
        DelayMs(15);
        SendAck();
        ExecuteInstructionPacket();
    }   
    else if(packetBuffer[2]==SENDLINKAGEBYTE){
        cobsInstructionBufferLength=decodeCOBS(packetBuffer,lastPacketSize,cobsInstructionBuffer);        
        if(!ValidateChecksum(18)) {
            DelayMs(15);
            SendNAck();
            return;
        }
        DelayMs(15);
        SendAck();
        ExecuteLinkagePacket();
    }
     else if(packetBuffer[2]==ACKBYTE){
        if(packetBuffer[1]==dfmID && packetBuffer[3]==dfmID){
            isAckReceived=1;
            return;
        }
    }
}

void ShortIntToUART2(int a){
  unsigned char c;
  RX485_ENABLE_SEND();
  c = a>>8;
  while (!UARTTransmitterIsReady(UART2));
       UARTSendDataByte(UART2, c);
  c = a;
  while (!UARTTransmitterIsReady(UART2));
       UARTSendDataByte(UART2, c); 
  RX485_DISABLE_SEND();
}

void IntToUART2(int a){
  unsigned char c;
  RX485_ENABLE_SEND();
  c = a>>24;
  while (!UARTTransmitterIsReady(UART2));
       UARTSendDataByte(UART2, c);
  c = a>>16;
  while (!UARTTransmitterIsReady(UART2));
       UARTSendDataByte(UART2, c);
  c = a>>8;
  while (!UARTTransmitterIsReady(UART2));
       UARTSendDataByte(UART2, c);
  c = a;
  while (!UARTTransmitterIsReady(UART2));
       UARTSendDataByte(UART2, c);
  RX485_DISABLE_SEND();
}

void StringToUART2(const char* buffer) {
    RX485_ENABLE_SEND();
    while (*buffer != '\0') {
        while (!UARTTransmitterIsReady(UART2));
        UARTSendDataByte(UART2, *buffer++);
    }
    RX485_DISABLE_SEND();
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
