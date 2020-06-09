#include "GlobalIncludes.h"

int counter;
int values[13][128];
int CurrentValues[13];
int volatile tmpValues[13];

unsigned char volatile analogUpdateFlag;


void FillCurrentStatus(struct StatusPacket *cS){
    int i,j;
    // want to use a trick here to speed things up
    unsigned char *statusPointer = &cS->W1VHigh;    
    j=0;
    for(i=0;i<13;i++) {
        *(statusPointer+j)=CurrentValues[i]>>16;
        *(statusPointer+j+1)=CurrentValues[i]>>8;
        *(statusPointer+j+2)=CurrentValues[i] & 0xFF;
        j+=3;
    }
}

void ClearAnalogValues(){
    int i,j;
     for (j = 0; j < 13; j++) {
      CurrentValues[j]=0;
      for (i = 0; i < 128; i++)
          values[j][i] = 0;
    }    
}
void ADC_EventHandler(uint32_t status) {   
    int blah;
    BLUELED_ON();
    if (analogUpdateFlag==1)
        YELLOWLED_ON();
    tmpValues[12]=ADC_ResultGet(ADC_RESULT_BUFFER_0); // Voltage
    tmpValues[0]=ADC_ResultGet(ADC_RESULT_BUFFER_1); //A1        
    tmpValues[2]=ADC_ResultGet(ADC_RESULT_BUFFER_2); //B1    
    tmpValues[1]=ADC_ResultGet(ADC_RESULT_BUFFER_3); // A2    
    tmpValues[3]=ADC_ResultGet(ADC_RESULT_BUFFER_4); // B2    
    tmpValues[10]=ADC_ResultGet(ADC_RESULT_BUFFER_5); // F1    
    tmpValues[7]=ADC_ResultGet(ADC_RESULT_BUFFER_6); // D2   
    tmpValues[11]=ADC_ResultGet(ADC_RESULT_BUFFER_7); // F2   
    tmpValues[4]=ADC_ResultGet(ADC_RESULT_BUFFER_8); // C1   
    tmpValues[8]=ADC_ResultGet(ADC_RESULT_BUFFER_9); // E1    
    tmpValues[5]=ADC_ResultGet(ADC_RESULT_BUFFER_10);  // C2  
    tmpValues[9]=ADC_ResultGet(ADC_RESULT_BUFFER_11); // E2   
    tmpValues[6]=ADC_ResultGet(ADC_RESULT_BUFFER_12); // D1   
    blah = ADC_ResultGet(ADC_RESULT_BUFFER_13);
    blah = ADC_ResultGet(ADC_RESULT_BUFFER_14);
    blah = ADC_ResultGet(ADC_RESULT_BUFFER_15);
    analogUpdateFlag=blah;
    analogUpdateFlag=1;   
    
    BLUELED_OFF();
}

/*
void ADC_EventHandler(uint32_t status) {    
    tmpValues[12]=ADC1BUF0; // Voltage
    tmpValues[0]=ADC1BUF1; //A1        
    tmpValues[2]=ADC1BUF2; //B1    
    tmpValues[1]=ADC1BUF3; // A2    
    tmpValues[3]=ADC1BUF4; // B2    
    tmpValues[10]=ADC1BUF5; // F1    
    tmpValues[7]=ADC1BUF6; // D2   
    tmpValues[11]=ADC1BUF7; // F2   
    tmpValues[4]=ADC1BUF8; // C1   
    tmpValues[8]=ADC1BUF9; // E1    
    tmpValues[5]=ADC1BUFA;  // C2  
    tmpValues[9]=ADC1BUFB; // E2   
    tmpValues[6]=ADC1BUFC; // D1   
           
    analogUpdateFlag=1;   
}
*/
 // THIS INFO WAS UPDATED FOR DFM V3.
    // If we sample 13 analog inputs 400 times per second, then we will be reading
    // 5200 times per second. Then we want 0.192ms max
    // per reading.  With continuous sampling we can get pretty close to this
    // because all of the heavy lifting is done by the peripheral in the
    // background.  We are not using main clocks to do it.

    // I will maximize the aquisition time because (I think) it will reduce
    // cross talk between readings.
    
    // Tad = 2 * (Tpb(ADCS+1)) where Tpb is the speed of the peripheral bus, i.e., 1/40000000.
    // The ADC requires 12 Tad for conversion.

    // With 31Tad sample time (SAMC=0x1F, which is maximum) and 12 Tad for conversion, the total Tad required
    // is 43.  We want this to be happening in about 0.2ms, so 1 Tad would need to be 4.5us. 
    // 1 Tpb = .025us, so the goal is 1 Tad = 180 Tpb. ADCS = 89 would give us this.
    // Thus, ADCS=89 decimal (0x59) would give 1Tad = 4.5us.
    
    // One conversion = 193.5us.  Thirteen conversions = 2.52ms.
    // 400 sets of 13 conversions = 1006ms.    

    // With O3 optimization, the duration of the interrupt is roughly 4.05us and
    // the time between interrupts is 2.14ms. 
    
    // The scope suggests that with these numbers we get a frequency of about
    // 466 hertz.  SAMPLE FREQUENCY IS THEREFORE 466HZ.
    
    // With a report executed 5 times per second, that implies that each report
    // will capture the previous 93 measures.
    
    // Because I am using a single buffer it is important to make sure
    // that I am able to unload the buffer before a full aquisition cycle
    // occurs. Otherwise, I may overwrite some values.
    // Timing the current interrupt, it takes roughly 4us.  One
    // conversion takes 193.5us, so we are okay.

void ConfigureAnalogInputs(){
    ClearAnalogValues();
    counter=0;      
    ADC_CallbackRegister(ADC_EventHandler, (uintptr_t)NULL);
}

void StepADC(){
    int i,j;  
    
    for(i=0;i<13;i++){
        j=tmpValues[i]; // Need this here to avoid interrupt changing it mid calculation.    
        CurrentValues[i]+=(j-values[i][counter]);  
        values[i][counter]=j; 
        if(CurrentValues[i]<0)
            CurrentValues[i]=0;              
    }
    
    counter++; 
    if(counter>=128) {
          counter=0;
          //PORTEINV=0x01;
    }
    
}