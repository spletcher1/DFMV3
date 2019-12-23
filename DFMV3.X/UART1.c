#include "GlobalIncludes.h"

//Actually in PIC32MX795F512L uC the UARTs numbering is like this one:

//UART1A = UART1
//UART2A = UART3
//UART3A = UART2
//UART1B = UART4
//UART2B = UART6
//UART3B = UART5

#define RX485_DELAY() DelayMs(1)


#define HEADER1_2 0xFF
#define HEADER3 0xFD

#define INSTRUCTIONPACKETSIZE 9 


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
//#define TARGET_BAUD_RATE  921600
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
    RX485_DELAY();    
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
	unsigned char data;
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
	}	
    while(DataRdyUART2()) {
		data=UARTGetDataByte(UART2);      
        if(isInPacket){
            packetBuffer[packetIndex++]=data;
            if(packetIndex+3>=INSTRUCTIONPACKETSIZE) {
                isPacketReceived = 1;              
                headerSum=0; // to avoid linking a header sequence across packets
                isInPacket=0;
            }                            
        }
        if(data==HEADER1_2){            
            headerSum+=data;
        }
        else if(data==HEADER3){            
            if(headerSum>=510){
                isInPacket=1;
                packetIndex=0;
            }
            headerSum=0;
        }
        else {
            headerSum=0;
        }      		
	}
    
    INTClearFlag(INT_U2RX);		
}

void CurrentStatusToUART2(struct StatusPacket *cs){
    unsigned char *statusPointer = (char *)&cs->Header1;
    int i;
    RX485_ENABLE_SEND();
    for(i=0;i<STATUSPACKETSIZE;i++){
        while (!UARTTransmitterIsReady(UART2));
            UARTSendDataByte(UART2, *(statusPointer+i));
    }
    RX485_DELAY();
    RX485_DISABLE_SEND();
}

void ProcessPacket() {  
         
    if(packetBuffer[0]!=dfmID && packetBuffer[0]!=255) return; // Packet not for me    
    if (isInDarkMode == 0) FLIP_GREEN_LED();
    switch(packetBuffer[1]){
        case 0x01: // Status Request
            DelayMs(5);
            CurrentStatusToUART2(GetStatusAtFront());            
            break;
        case 0x02: // Set Darkmode               
            SetDarkMode(packetBuffer[2]);
            break;
        case 0x03: // Set Optostate    
            SetOptoState(packetBuffer[2],packetBuffer[3]);
            break;
        case 0x04: // Set Hertz
            SetHertz((packetBuffer[2]<<8) + packetBuffer[3]);
            break;
        case 0x05: // Set Pulsewidth
            SetPulseWidth_ms((packetBuffer[2]<<8) + packetBuffer[3]);
            break;
        case 0x06: // Return ID
            DelayMs(5);
            CharToUART2(dfmID);
            break;
        default:
            break;        
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
  RX485_DELAY();
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
  RX485_DELAY();     
  RX485_DISABLE_SEND();
}

void StringToUART2(const char* buffer) {
    RX485_ENABLE_SEND();
    while (*buffer != '\0') {
        while (!UARTTransmitterIsReady(UART2));
        UARTSendDataByte(UART2, *buffer++);
    }
    RX485_DELAY();
    RX485_DISABLE_SEND();
}
