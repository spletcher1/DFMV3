#include "GlobalIncludes.h"

#define HEADER1_2 0xFF
#define HEADER3 0xFD

#define INSTRUCTIONPACKETSIZE 6

unsigned char volatile isPacketReceived;
unsigned char packetBuffer[250];
unsigned char volatile packetIndex;

unsigned char currentError;

unsigned char volatile isInPacket;
unsigned char volatile byteCountDown;
unsigned int volatile headerSum;
struct StatusPacket currentStatus; 

extern unsigned char dfmID;
extern unsigned char isInDarkMode;
extern unsigned char volatile OptoState1;
extern unsigned char volatile OptoState2;


unsigned char currentTemperature;
unsigned char currentHumidity;
unsigned int currentLUX;


/////////////////////////////////////////////////////////////////
// Select one to specify configuration.
// Baud Rate 19200
// Baud rate 38400
// Baud rate 115200
#define TARGET_BAUD_RATE  921600

/////////////////////////////////////////////////////////////////
void CharToUART1(char c){
        while (!UARTTransmitterIsReady(UART1));
        UARTSendDataByte(UART1, c);
}

void ConfigureUART1Interrupts(){
	// For now we interrupt on RX
	INTSetVectorPriority(INT_UART_1_VECTOR,INT_PRIORITY_LEVEL_3);
	INTClearFlag(INT_U1RX);
	INTEnable(INT_U1RX,INT_ENABLED);
	INTEnable(INT_U1E,INT_ENABLED);
}

void inline ClearPacketBuffer(){
    packetIndex=0;
    isInPacket=0;
    headerSum=0;   
}

void ConfigureUART1(void) {
    // Note: As of now, the baud rate set for the parallax RFID reader is 2400.
    // Data bits = 8; no parity; stop bits = 1;

    UARTConfigure(UART1, UART_ENABLE_HIGH_SPEED);
    UARTSetFifoMode(UART1, UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART1, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART1, GetPeripheralClock(), TARGET_BAUD_RATE);
    UARTEnable(UART1,UART_ENABLE_FLAGS(UART_ENABLE | UART_TX | UART_RX | UART_PERIPHERAL));

    ConfigureUART1Interrupts();
    ClearPacketBuffer(); 
    currentStatus.Header1=0xFF;
    currentStatus.Header2=0xFF;
    currentStatus.Header3=0xFD;   
    currentStatus.ID=dfmID;
    currentError=0;
    
    currentTemperature=25;
    currentHumidity = 40;
    currentLUX=1234;
    
}



void __ISR(_UART1_VECTOR, IPL3AUTO) UART1Interrupt(void){
	int error;
	unsigned char data;
	error = UART1GetErrors();	
	if (error > 0) {
		if (error & 0x01) { //Overflow Error
			U1STAbits.OERR = 0;
			mU1EClearIntFlag();
			ClearPacketBuffer();
            INTClearFlag(INT_U1RX);	
            StringToUART1("Error");
            currentError=1;
            return;
		}
		else if(error & 0x02) {
			U1STAbits.FERR=0;
			ClearPacketBuffer();
			StringToUART1("Error");
			mU1EClearIntFlag();
            currentError=1;
            INTClearFlag(INT_U1RX);	
            return;
		}	
	}
	currentError=0;
    while(DataRdyUART1()) {
		data=UARTGetDataByte(UART1);      
        if(isInPacket){
            packetBuffer[packetIndex++]=data;
            if(packetIndex>=INSTRUCTIONPACKETSIZE) {
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
    INTClearFlag(INT_U1RX);		
}

void CurrentStatusToUART1(){
    unsigned char *statusPointer = (char *)&currentStatus.Header1;
    int i;
    for(i=0;i<38;i++)
        CharToUART1(*(statusPointer+i));
        
}

void ProcessPacket() {        
    if(packetBuffer[0]!=dfmID && packetBuffer[0]!=255) return; // Packet not for me
    switch(packetBuffer[1]){
        case 0x01: // Status Request
            currentStatus.ErrorFlag = currentError;
            FillCurrentStatus();
            currentStatus.Optostate1 = OptoState1;
            currentStatus.Optostate2 = OptoState2;
            currentStatus.DarkMode = isInDarkMode;
            currentStatus.Humidity = currentHumidity;
            currentStatus.Temperature = currentTemperature;
            currentStatus.LightHigh = currentLUX>>8;
            currentStatus.LightLow = currentLUX & 0xFF;
            currentStatus.CRCHigh=0x01;
            currentStatus.CRCLow=0x01;
            CurrentStatusToUART1();
            break;
        case 0x02: // Set Darkmode               
            SetDarkMode(packetBuffer[2]);
            break;
        case 0x03: // Set Optostate    
            SetOptoState(packetBuffer[2],packetBuffer[3]);
            break;
        case 0x04: // Set Hertz
            SetHertz((packetBuffer[2]<<8) + packetBuffer[1]);
            break;
        case 0x05: // Set Pulsewidth
            SetPulseWidth_ms((packetBuffer[2]<<8) + packetBuffer[1]);
            break;
        default:
            break;        
    }
}



void ShortIntToUART1(int a){
  unsigned char c;
  c = a>>8;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c);
  c = a;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c); 
}

void IntToUART1(int a){
  unsigned char c;
  c = a>>24;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c);
  c = a>>16;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c);
  c = a>>8;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c);
  c = a;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c);
}

void StringToUART1(const char* buffer) {
    while (*buffer != '\0') {
        while (!UARTTransmitterIsReady(UART1));
        UARTSendDataByte(UART1, *buffer++);
    }
}
