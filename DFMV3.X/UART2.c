#include "GlobalIncludes.h"

//Actually in PIC32MX795F512L uC the UARTs numbering is like this one:

//UART1A = UART1
//UART2A = UART3
//UART3A = UART2
//UART1B = UART4
//UART2B = UART6
//UART3B = UART5

#define MAXPACKETS 15
#define HEADER1_2 0xFF
#define HEADER3 0xFD // This indicates a instruction packet
#define HEADER3_2 0xFC // This indicates a status request packet
#define HEADER3_3 0xFE // This indicates a status request packet

#define INSTRUCTIONPACKETSIZE 40 // Not including headers 
#define STATUSREQUESTPACKETSIZE 2 // Not including headers 

#define COBSBUFFERSIZE STATUSPACKETSIZE*MAXPACKETS

unsigned char cobsBuffer[COBSBUFFERSIZE];
unsigned char preCodedBuffer[COBSBUFFERSIZE];
unsigned int cobsBufferLength;
extern int bufferSize;

unsigned char volatile isPacketReceived;
unsigned char volatile packetBuffer[250];
unsigned char volatile packetIndex;

unsigned char volatile isInPacket;
unsigned int volatile headerSum;

extern unsigned char dfmID;
extern unsigned char isInDarkMode;
extern errorFlags_t volatile currentError;

/////////////////////////////////////////////////////////////////
// Select one to specify configuration.
// Baud Rate 19200
// Baud rate 38400
// Baud rate 115200
// #define TARGET_BAUD_RATE  921600
#define TARGET_BAUD_RATE  115200

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
	INTSetVectorPriority(INT_UART_2_VECTOR,INT_PRIORITY_LEVEL_4);
	INTClearFlag(INT_U2RX);
    INTClearFlag(INT_U2TX);    
	INTEnable(INT_U2RX,INT_ENABLED);
	INTEnable(INT_U2E,INT_ENABLED);
}

void inline ClearPacketBuffer(){
    packetIndex=0;
    isInPacket=0;
    headerSum=0;   
}

void ConfigureUART2(void) {
    // Note: As of now, the baud rate set for the parallax RFID reader is 2400.
    // Data bits = 8; no parity; stop bits = 1;

    UARTConfigure(UART2, UART_ENABLE_HIGH_SPEED);
    UARTSetFifoMode(UART2, UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART2, GetPeripheralClock(), TARGET_BAUD_RATE);
    UARTEnable(UART2,UART_ENABLE_FLAGS(UART_ENABLE | UART_TX | UART_RX | UART_PERIPHERAL));

    ConfigureUART2Interrupts();
    ClearPacketBuffer(); 
       
}

// Interestingly, this interrupt seems to be called 9 times to process
// a 9 Byte status packet!  The duration of each interrupt call is about 1us, with
// O3 optimization.

// Additional timings revealed that it takes about 570us to receive the status request
// and then complete sending the response package. This seems well within the needed
// interval. This timing could easily handle even 100 DFM.
void __ISR(_UART2_VECTOR, IPL4AUTO) UART2Interrupt(void){
	int error;
	unsigned char data,tmp;
	error = UART2GetErrors();	
	if (error > 0) {
		if (error & 0x01) { //Overflow Error
			U2STAbits.OERR = 0;
			INTClearFlag(INT_U2E);
			ClearPacketBuffer();
            INTClearFlag(INT_U2RX);	           
            currentError.bits.UART=1;
            return;
		}
		else if(error & 0x02) {
			U2STAbits.FERR=0;
			ClearPacketBuffer();			
			INTClearFlag(INT_U2E);
            currentError.bits.UART=1;
            INTClearFlag(INT_U2RX);	
            return;
		}	
        else {
            currentError.bits.TBD3=1;
            while(DataRdyUART2())            
                data = UARTGetDataByte(UART2);            
            UART2ClearAllErrors();
            ClearPacketBuffer();
            INTClearFlag(INT_U2E);
            INTClearFlag(INT_U2RX);	
        }
	}	
    while(DataRdyUART2()) {
		data=UARTGetDataByte(UART2);      
        if(isInPacket){
            packetBuffer[packetIndex++]=data;
            tmp=packetBuffer[0];
            if(packetBuffer[0]==HEADER3_2 || packetBuffer[0]==HEADER3_3){ // Status Request
                if(packetIndex>STATUSREQUESTPACKETSIZE){
                    isPacketReceived=1;
                    headerSum=0;
                    isInPacket=0;
                }                    
            }
            else if(packetBuffer[0]==HEADER3){
                if(packetIndex>INSTRUCTIONPACKETSIZE) {
                    isPacketReceived = 1;              
                    headerSum=0; // to avoid linking a header sequence across packets
                    isInPacket=0;
                }
            }                                        
            else {
                currentError.bits.TBD3=1;
            }
            
        }
        if(data==HEADER1_2){            
            headerSum+=data;
        }
        else if(data==HEADER3 || data==HEADER3_2 || data==HEADER3_3){            
            if(headerSum>=510){
                isInPacket=1;
                // Need to keep the header3 byte because
                // it now says what kind of packet it is.
                packetBuffer[0]=data;
                packetIndex=1;
            }
            headerSum=0;
        }
        else {
            headerSum=0;
        }      		
	}
    
    INTClearFlag(INT_U2RX);		
}

void CurrentStatusPacketSetToUART2(int numPacketsToSend){
    struct StatusPacket *cs1;
    unsigned char *statusPointer;
    cs1 = GetNextStatusInLine();
    statusPointer= (char *)&cs1->Header1;
    int i,j,counter=0;
    
    for(i=0;i<STATUSPACKETSIZE;i++){
        preCodedBuffer[counter]=*(statusPointer+i);        
        counter++;        
    }    
    
    for(j=1;j<numPacketsToSend;j++){
        cs1 = GetNextStatusInLine();
        statusPointer = (char *)&cs1->ErrorFlag;
        for(i=0;i<NOHEADERSPSIZE;i++){
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
}


void CurrentStatusToUART2(struct StatusPacket *cs){
    unsigned char *statusPointer = (char *)&cs->Header1;
    int i;
    RX485_ENABLE_SEND();
    for(i=0;i<STATUSPACKETSIZE;i++){
        while (!UARTTransmitterIsReady(UART2));
            UARTSendDataByte(UART2, *(statusPointer+i));
    }    
    RX485_DISABLE_SEND();
}

void SendAck(){
    CharToUART2(dfmID);
}

void SendNAck(){
    CharToUART2(0xFE);
}

unsigned char ValidateChecksum(){    
    int i;
    unsigned int checksum=0,actual;
    for(i=1;i<37;i++)
        checksum+=packetBuffer[i];
    checksum = (checksum ^ 0xFFFFFFFF)+1;
    
    actual=(unsigned int)(packetBuffer[37]<<24);
    actual+=(unsigned int)(packetBuffer[38]<<16);
    actual+=(unsigned int)(packetBuffer[39]<<8);
    actual+=(unsigned int)(packetBuffer[40]);
    // Checksum is calculated excluding the header characters.    
    return (checksum==actual);
}

void ExecuteInstructionPacket(){
    unsigned char index=2,i;
    unsigned int freq,pw,decay,delay,maxTime;
    int thresh[12];
    if(packetBuffer[index++]==0)
        SetDarkMode(0);
    else
        SetDarkMode(1);
    
    freq = (unsigned int)(packetBuffer[index]<<8)+(unsigned int)(packetBuffer[index+1]);
    index+=2;
    pw = (unsigned int)(packetBuffer[index]<<8)+(unsigned int)(packetBuffer[index+1]);
    index+=2;
    decay = (unsigned int)(packetBuffer[index]<<8)+(unsigned int)(packetBuffer[index+1]);
    index+=2;
    delay = (unsigned int)(packetBuffer[index]<<8)+(unsigned int)(packetBuffer[index+1]);
    index+=2;
    maxTime = (unsigned int)(packetBuffer[index]<<8)+(unsigned int)(packetBuffer[index+1]);
    index+=2;   
    for(i=0;i<12;i++){
        thresh[i]=(unsigned int)(packetBuffer[index]<<8)+(unsigned int)(packetBuffer[index+1]);
        index+=2;
    }
    SetOptoParameters(freq,pw);
    SetLEDParams(decay,delay,maxTime);
    SetLEDThresholds(thresh);                   
}

void ProcessPacket() {           
    int num;
    if(packetBuffer[1]!=dfmID && packetBuffer[1]!=255) return; // Packet not for me    
    if (isInDarkMode == 0) FLIP_GREEN_LED();
    if(packetBuffer[0]==HEADER3_2){
        if(packetBuffer[1]==dfmID && packetBuffer[2]==dfmID){
            DelayMs(15);
            if(bufferSize>MAXPACKETS)
                num=MAXPACKETS;
            else
                num = bufferSize;
            if(num>0)
                CurrentStatusPacketSetToUART2(num);                        
        }
    }
    else if(packetBuffer[0]==HEADER3_3){
        if(packetBuffer[1]==dfmID && packetBuffer[2]==dfmID){
            InitializeStatusPacketBuffer();
            DelayMs(15);
            SendAck();
        }
    }
    else if(packetBuffer[0]==HEADER3){
        if(!ValidateChecksum()) {
            DelayMs(15);
            SendNAck();
            return;
        }
        DelayMs(15);
        SendAck();
        ExecuteInstructionPacket();
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

unsigned int decodeCOBS(unsigned char* encodedBuffer,unsigned int bytesToEncode, unsigned char* decodedBuffer)
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
