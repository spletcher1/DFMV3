#include "GlobalIncludes.h"

#define HEADER1_2 0xFF
#define HEADER3 0xFD

#define INSTRUCTIONPACKETSIZE 6
#define STATUSPACKETSIZE 65

unsigned char volatile isPacketReceived;
unsigned char packetBuffer[250];
unsigned char volatile packetIndex;

unsigned char volatile isInPacket;
unsigned char volatile byteCountDown;
unsigned int volatile headerSum;
struct StatusPacket currentStatus; 

extern errorFlags_t currentError;

extern unsigned char dfmID;
extern unsigned char isInDarkMode;
extern unsigned char volatile OptoState1;
extern unsigned char volatile OptoState2;
extern unsigned int volatile pulseWidth_ms;
extern unsigned int volatile hertz;

extern unsigned int TSL2591_LUX;
extern unsigned int Si7021_Humidity;
extern unsigned int Si7021_Temperature;


void inline ClearPacketBuffer(){
    packetIndex=0;
    isInPacket=0;
    headerSum=0;   
}

void ConfigureUART1(void) {
    ClearPacketBuffer(); 
    currentStatus.Header1=0xFF;
    currentStatus.Header2=0xFF;
    currentStatus.Header3=0xFD;   
    currentStatus.ID=dfmID;          
}

// Interestingly, this interrupt seems to be called 9 times to process
// a 9 Byte status packet!  The duration of each interrupt call is about 1us, with
// O3 optimization.

// Additional timings revealed that it takes about 570us to receive the status request
// and then complete sending the response package. This seems well within the needed
// interval. This timing could easily handle even 100 DFM.
void UART1_Callback(){
	int error;
	unsigned char data;
	error = UART1_ErrorGet();
	if (error > 0) {
		if (error & 0x01) { //Overflow Error			
			ClearPacketBuffer();            
            currentError.bits.UART=1;
            return;
		}
		else if(error & 0x02) {			
			ClearPacketBuffer();						
            currentError.bits.UART=1;            
            return;
		}	
	}	
    while((U1STA & _U1STA_URXDA_MASK) == _U1STA_URXDA_MASK) {
		data=UART1_ReadByte();      
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
    
    //INTClearFlag(INT_U1RX);		
}

void CurrentStatusToUART1(){
    unsigned char *statusPointer = (unsigned char *)&currentStatus.Header1;
    int i;
    for(i=0;i<STATUSPACKETSIZE;i++)
        CharToUART1(*(statusPointer+i));        
}

void ProcessPacket() {  
    unsigned int checksum=0,i;
    unsigned char *statusPointer = (unsigned char *)&currentStatus.Header1;
    if(packetBuffer[0]!=dfmID && packetBuffer[0]!=255) return; // Packet not for me
    if (isInDarkMode == 0) FLIP_GREEN_LED();
    switch(packetBuffer[1]){
        case 0x01: // Status Request
            currentStatus.ErrorFlag = currentError.byte;
            //FillCurrentStatus();
            currentStatus.Optostate1 = OptoState1;
            currentStatus.Optostate2 = OptoState2;
            currentStatus.OptoFreq1 = hertz>>8;
            currentStatus.OptoFreq2 = hertz & 0xFF;
            currentStatus.OptoPW1 = pulseWidth_ms >> 8;
            currentStatus.OptoPW2 = pulseWidth_ms & 0xFF;
            currentStatus.DarkMode = isInDarkMode;
            currentStatus.Temperature1 = (Si7021_Temperature >> 24);
            currentStatus.Temperature2 = ((Si7021_Temperature >> 16) & 0xFF);
            currentStatus.Temperature3 = ((Si7021_Temperature >> 8) & 0xFF);
            currentStatus.Temperature4 = (Si7021_Temperature & 0xFF);            
            currentStatus.Humidity1 = (Si7021_Humidity >> 24);
            currentStatus.Humidity2 = ((Si7021_Humidity >> 16) & 0xFF);
            currentStatus.Humidity3 = ((Si7021_Humidity >> 8) & 0xFF);
            currentStatus.Humidity4 = (Si7021_Humidity & 0xFF);
            currentStatus.LightHigh = TSL2591_LUX>>8;
            currentStatus.LightLow = TSL2591_LUX & 0xFF;
            for(i=0;i<(STATUSPACKETSIZE-4);i++) checksum+= *(statusPointer+i);
            checksum = (checksum ^ 0xFFFFFFFF) + 0x01;
            currentStatus.Checksum1=checksum>>24;
            currentStatus.Checksum2=(checksum>>16) & 0xFF;            
            currentStatus.Checksum3=(checksum>>8) & 0xFF;
            currentStatus.Checksum4=checksum & 0xFF;
            CurrentStatusToUART1();
            currentError.byte=0x00;
            break;
        case 0x02: // Set Darkmode               
            //SetDarkMode(packetBuffer[2]);
            break;
        case 0x03: // Set Optostate    
            //SetOptoState(packetBuffer[2],packetBuffer[3]);
            break;
        case 0x04: // Set Hertz
            //SetHertz((packetBuffer[2]<<8) + packetBuffer[3]);
            break;
        case 0x05: // Set Pulsewidth
            //SetPulseWidth_ms((packetBuffer[2]<<8) + packetBuffer[3]);
            break;
        case 0x06: // Return ID
            //CharToUART1(dfmID);
            break;
        default:
            break;        
    }
    
}


