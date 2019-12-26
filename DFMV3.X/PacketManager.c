#include "GlobalIncludes.h"

// This has to be small or else when we start
// pulling data, we will be pulling from too far in 
// the past.  Alternatively, we can write a signal
// to reset the buffer just before 
#define RINGBUFFERSIZE 10


struct StatusPacket statusBuffer[RINGBUFFERSIZE]; 
unsigned char head = 0, tail = 0;
int bufferSize=0;
unsigned char appendedStatusPackets[309];

struct StatusPacket emptyPacket;

extern errorFlags_t volatile currentError;

extern unsigned char dfmID;
extern unsigned char isInDarkMode;
extern unsigned char volatile OptoState1;
extern unsigned char volatile OptoState2;
extern unsigned int volatile pulseWidth_ms;
extern unsigned int volatile hertz;

extern unsigned int TSL2591_LUX;
extern unsigned int Si7021_Humidity;
extern unsigned int Si7021_Temperature;


int counter100ms;
unsigned int recordCounter;

void inline FillChecksum(struct StatusPacket *tmp){
    unsigned int checksum=0;
    unsigned char *statusPointer;
    int i;
    statusPointer = (char *)&tmp->Header1;
    for (i = 4; i < (STATUSPACKETSIZE - 4); i++) checksum += *(statusPointer + i);
    checksum = (checksum ^ 0xFFFFFFFF) + 0x01;
    tmp->Checksum1 = checksum >> 24;
    tmp->Checksum2 = (checksum >> 16) & 0xFF;
    tmp->Checksum3 = (checksum >> 8) & 0xFF;
    tmp->Checksum4 = checksum & 0xFF;  
}


void FillEmptyPacket(){    
    errorFlags_t tmpe;
    int i,j;
    unsigned char *statusPointer;
    tmpe.bits.STATUSBUFFER=1;
    
    emptyPacket.Header1=0xFF;
    emptyPacket.Header2=0xFF;
    emptyPacket.Header3=0xFD;   
    emptyPacket.ID=dfmID;     
    emptyPacket.ErrorFlag = tmpe.byte;   
    
    statusPointer = &emptyPacket.W1VHigh;    
    j=0;
    for(i=0;i<13;i++) {
        *(statusPointer+j)=0;
        *(statusPointer+j+1)=0;
        *(statusPointer+j+2)=0;
        j+=3;
    }
        
    emptyPacket.Optostate1 = 0;
    emptyPacket.Optostate2 = 0;
    emptyPacket.OptoFreq1 = 0;
    emptyPacket.OptoFreq2 = 0;
    emptyPacket.OptoPW1 = 0;
    emptyPacket.OptoPW2 = 0;
    emptyPacket.DarkMode = 0;
    emptyPacket.Temperature1 = 0;
    emptyPacket.Temperature2 = 0;
    emptyPacket.Temperature3 = 0;
    emptyPacket.Temperature4 = 0;   
    emptyPacket.Humidity1 = 0;
    emptyPacket.Humidity2 = 0;
    emptyPacket.Humidity3 = 0;
    emptyPacket.Humidity4 = 0;
    emptyPacket.LightHigh = 0;
    emptyPacket.LightLow = 0;      
    emptyPacket.Index1 = 0;
    emptyPacket.Index2 = 0;
    emptyPacket.Index3 = 0;
    emptyPacket.Index4 = 0;
    FillChecksum(&emptyPacket);      
}


void InitializeStatusPacketBuffer(){   
    counter100ms=0;
    recordCounter=0;
    FillEmptyPacket();
    // We set this to avoid error on first use.           
}

struct StatusPacket *GetNextStatusInLine(){    
    struct StatusPacket* tmp;

    if(bufferSize==0)
        return &emptyPacket;
    
    tmp=&statusBuffer[tail];
    bufferSize--;
    tail++;
    if(tail>=RINGBUFFERSIZE)
        tail=0;   
  
    return tmp;
}

void AddCurrentStatus() {    
    unsigned char *statusPointer;
    
    statusBuffer[head].Header1=0xFF;
    statusBuffer[head].Header2=0xFF;
    statusBuffer[head].Header3=0xFD;   
    statusBuffer[head].ID=dfmID;     
    statusBuffer[head].ErrorFlag = currentError.byte;
    statusPointer = (char *)&statusBuffer[head].Header1;  
    FillCurrentStatus(&statusBuffer[head]);
    statusBuffer[head].Optostate1 = OptoState1;
    statusBuffer[head].Optostate2 = OptoState2;
    statusBuffer[head].OptoFreq1 = hertz >> 8;
    statusBuffer[head].OptoFreq2 = hertz & 0xFF;
    statusBuffer[head].OptoPW1 = pulseWidth_ms >> 8;
    statusBuffer[head].OptoPW2 = pulseWidth_ms & 0xFF;
    statusBuffer[head].DarkMode = isInDarkMode;
    statusBuffer[head].Temperature1 = (Si7021_Temperature >> 24);
    statusBuffer[head].Temperature2 = ((Si7021_Temperature >> 16) & 0xFF);
    statusBuffer[head].Temperature3 = ((Si7021_Temperature >> 8) & 0xFF);
    statusBuffer[head].Temperature4 = (Si7021_Temperature & 0xFF);
    statusBuffer[head].Humidity1 = (Si7021_Humidity >> 24);
    statusBuffer[head].Humidity2 = ((Si7021_Humidity >> 16) & 0xFF);
    statusBuffer[head].Humidity3 = ((Si7021_Humidity >> 8) & 0xFF);
    statusBuffer[head].Humidity4 = (Si7021_Humidity & 0xFF);   
    statusBuffer[head].LightHigh = TSL2591_LUX >> 8;
    statusBuffer[head].LightLow = TSL2591_LUX & 0xFF;   
    statusBuffer[head].Index1 = (recordCounter >> 24);
    statusBuffer[head].Index2 = ((recordCounter >> 16) & 0xFF);
    statusBuffer[head].Index3 = ((recordCounter >> 8) & 0xFF);
    statusBuffer[head].Index4 = (recordCounter & 0xFF);    
       
    bufferSize++;
    if(bufferSize>RINGBUFFERSIZE) {
        statusBuffer[head].ErrorFlag = statusBuffer[head].ErrorFlag | 0x40; 
        bufferSize=RINGBUFFERSIZE;        
        tail++;
        if(tail>=RINGBUFFERSIZE)
            tail=0;   
    }
    // This is after in case the error flag is changed.
    FillChecksum(&statusBuffer[head]);    
        
    head++;
    if(head>=RINGBUFFERSIZE)
        head=0;       
    recordCounter++;
    currentError.byte = 0x00;    
}

// This function is meant to be called every 100ms
void StepPacketManager(){
    if(++counter100ms>=2){
        AddCurrentStatus();
        counter100ms=0;
    }    
}