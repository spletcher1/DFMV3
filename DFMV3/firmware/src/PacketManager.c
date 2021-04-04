#include "GlobalIncludes.h"

#define RINGBUFFERSIZE 1500

struct StatusPacket statusBuffer[RINGBUFFERSIZE]; 
unsigned int head = 0, tail = 0,tailPlaceHolder=0;
unsigned int bufferSize=0;
 
struct StatusPacket emptyPacket;
struct StatusPacket *lastPacket;

extern errorFlags_t volatile currentError;

extern unsigned char dfmID;
extern unsigned char isInDarkMode;
extern unsigned int volatile OptoState;
extern unsigned int volatile pulseWidth_ms;
extern unsigned int volatile hertz;

unsigned int recordCounter;

unsigned char tempError;

void inline FillChecksum(struct StatusPacket *tmp){
    unsigned int checksum=0;
    unsigned char *statusPointer;
    int i;
    statusPointer = (unsigned char *)&tmp->ID;
    for (i = 0; i < (STATUSPACKETSIZE - 4); i++) checksum += *(statusPointer + i);
    checksum = (checksum ^ 0xFFFFFFFF) + 0x01;
    tmp->Checksum1 = checksum >> 24;
    tmp->Checksum2 = (checksum >> 16) & 0xFF;
    tmp->Checksum3 = (checksum >> 8) & 0xFF;
    tmp->Checksum4 = checksum & 0xFF;  
}


void FillEmptyPacket(){        
    int i,j;
    unsigned char *statusPointer;    
         
    emptyPacket.ID=dfmID;     
    emptyPacket.ErrorFlag = 0;   
    
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
    emptyPacket.Index1 = 0;
    emptyPacket.Index2 = 0;
    emptyPacket.Index3 = 0;
    emptyPacket.Index4 = 0;
    FillChecksum(&emptyPacket);      
}


void InitializeStatusPacketBuffer(){       
    recordCounter=1;
    bufferSize=0;
    head=tail=tailPlaceHolder=0;
    FillEmptyPacket();
    // Clear any packet error
    currentError.byte = currentError.byte & 0xFB;
    // We set this to avoid error on first use.       
}

void inline SetTailPlaceHolder(){
    tailPlaceHolder=tail;
}

void inline ResetTail(){       
    int diff;
    if(tail>=tailPlaceHolder) // didn't go around ring buffer since last ack
        diff = tail - tailPlaceHolder;
    else
        diff = tail + (RINGBUFFERSIZE-tailPlaceHolder);
               
    tail = tailPlaceHolder;
    bufferSize+=diff;
    if(bufferSize>RINGBUFFERSIZE) {                  
        bufferSize=RINGBUFFERSIZE;        
        tail++;
        if(tail>=RINGBUFFERSIZE){
            tail=0;               
        }
    }
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

struct StatusPacket *GetLastestStatus(){    
 
    if(bufferSize==0)
        return &emptyPacket;
       
    return lastPacket;
}

void AddCurrentStatus() {            
    
    statusBuffer[head].ID=dfmID;     
    statusBuffer[head].ErrorFlag = currentError.byte;    
    FillCurrentStatus(&statusBuffer[head]);
    statusBuffer[head].Optostate2 = OptoState & 0xFF;
    statusBuffer[head].Optostate1 = (OptoState >> 8) & 0xFF;
    statusBuffer[head].OptoFreq1 = hertz >> 8;
    statusBuffer[head].OptoFreq2 = hertz & 0xFF; 
    statusBuffer[head].OptoPW1 = pulseWidth_ms >> 8;
    statusBuffer[head].OptoPW2 = pulseWidth_ms & 0xFF;
    statusBuffer[head].DarkMode = isInDarkMode;
    //statusBuffer[head].DarkMode = tempError;
    statusBuffer[head].Index1 = (recordCounter >> 24);
    statusBuffer[head].Index2 = ((recordCounter >> 16) & 0xFF);
    statusBuffer[head].Index3 = ((recordCounter >> 8) & 0xFF);
    statusBuffer[head].Index4 = (recordCounter & 0xFF);    
           
    // This is after in case the error flag is changed.
    FillChecksum(&statusBuffer[head]);       
    lastPacket = &statusBuffer[head];
    head++;
    if(head>=RINGBUFFERSIZE)
        head=0;       
    recordCounter++;
    if(bufferSize>=RINGBUFFERSIZE) {                  
        bufferSize=RINGBUFFERSIZE;        
        tail++;
        if(tail>=RINGBUFFERSIZE){
            tail=0;               
        }
    }
    else {
        bufferSize++;
    }
    
    currentError.byte = 0;       
}

// This function is meant to be called every 100ms
void StepPacketManager(){   
    AddCurrentStatus();         
}