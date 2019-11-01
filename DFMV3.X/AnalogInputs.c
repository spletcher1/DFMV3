#include "GlobalIncludes.h"

/**********************************************
 * For DFMV3, here is the relationship between analog
 * inputs and wells
 * A1 = AN2
 * A2 = AN4
 * B1 = AN3
 * B2 = AN5
 * C1 = AN11
 * C2 = AN13
 * D1 = AN15
 * D2 = AN9
 * E1 = AN12
 * E2 = AN14
 * F1 = AN8
 * F2 = AN10
 * 
 * The Volts in reading is AN0
 * 
 * 
 *
 */
int volatile counter;
int volatile values[13][128];
int volatile CurrentValues[13];

extern struct StatusPacket currentStatus;
unsigned char volatile analogUpdateFlag;

void FillCurrentStatus(){
    int i,j;
    // want to use a trick here to speed things up
    unsigned char *statusPointer = &currentStatus.W1High;    
    j=0;
    for(i=0;i<13;i++) {
        *(statusPointer+j)=CurrentValues[i]>>8;
        *(statusPointer+j+1)=CurrentValues[i] & 0xFF;
        j+=2;
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

void ConfigureScanningAnalogInputs(){
    // COnfigure all analog inputs as such
    AD1PCFG = 0x0000;
    TRISB = 0xFFFF;

    TRISECLR = 0x01;
    
    AD1CHSbits.CH0NA = 0; // Negative input is Vr-
    Nop();Nop();
    AD1CHSbits.CH0NB = 0; // Negative input is Vr-
    Nop();Nop();
    AD1CON1bits.FORM = 0x04; //32-bit integer output
    Nop();Nop();
    AD1CON1bits.SSRC = 0x07;  // Autoconvert, ie. start conversion directly after sampling.
    Nop();Nop();
    AD1CON1bits.ASAM = 0 ; // This enables manual start of sampling by setting SAMP.  For now, disable it.
    Nop();Nop();
    AD1CON2bits.VCFG = 0x00;  // Use AVdd and AVss as positive and negative voltage, respecively.
    Nop();Nop();
    // AD1CHSbits.CH0SA = 0x03; // No need to link to MUX A because we will autosample.
    Nop();Nop();
    AD1CON2bits.CSCNA = 1; // Do not scan inputs.,
     Nop();Nop();
   
    // Need to make the buffer be composed of one 16 bit buffer.  This will
    // allow me to update the contents in one buffer while readings are still being loaded into the second.
    AD1CON2bits.BUFM = 0;

    Nop();Nop();
    AD1CON2bits.SMPI = 0x0D; // Make the buffer thirteen entries (12 wells plus the volts in input).
    Nop();Nop();
    AD1CON2bits.ALTS = 0; // Always sample from MUX A (do not alternate between A and B).
    Nop();Nop();

    // The following analog inputs are used
    // AIIndex[12] = {0,2,3,4,5,8,9,10,11,12,13,14,15};
    // Set the appropriate bits in AD1CSSL
    AD1CSSL =0xFF3D;

    AD1CON3bits.ADRC=0; // Use peripheral bus as ADC clock (won't work in sleep mode).
    Nop();Nop();
    // If we sample 13 analog inputs 400 times per second, then we will be reading
    // 6000 times per second. Then we want 0.16ms max
    // per reading.  With continuous sampling we can get pretty close to this
    // because all of the heavy lifting is done by the peripheral in the
    // background.  We are not using main clocks to do it.

    // I will maximize the aquisition time because (I think) it will reduce
    // cross talk between readings.

    // With 31Tad sample time (SAMC=0x1F) and 12 Tad for conversion, the total Tad required
    // is 43.  We want this to be happening in 0.2ms, so 1 Tad is 4us. 4us is about 186 Tpb.
    // To be safe, we will take this down to set Tad = 150 Tpb.
    // Thus, ADCS=74 decimal (0x4a) would give 1Tad = 3.75us.
    // One conversion = 161.25us.  Thirteen conversions = 2.09ms.
    // 500 sets of 12 conversions = 1048ms.
    // For some reason, when timing the rate of the interrupt, it is called
    // about every 2.1ms, which is a bit less often then predicted.
    // I am not exactly sure why at this moment.

    // Nevertheless, if each report is called 200ms, it will encompass roughly the past 100 measures.
    // Indeed, timing the duration between every 100 reads results in something just
    // over 200ms.

    // Because I am using a single buffer it is important to make sure
    // that I am able to unload the buffer before a full aquisition cycle
    // occurs. Otherwise, I may overwrite some values.
    // Timing the current interrupt, it takse roughly 3.5us.  One
    // conversion takes 161.25 us, so we are okay.

    AD1CON3bits.SAMC = 0x1F;
    Nop();Nop();  
    AD1CON3bits.ADCS = 0x4A;  
     Nop();Nop();   
}

void StartContinuousSampling(){
    int i;
    for(i=0;i<12;i++)
        CurrentValues[i]=i;
    return;
    ClearAnalogValues();
    ConfigureScanningAnalogInputs();
    AD1CON1SET = 0x0004; // Set auto.
    AD1CON1SET = 0x8000; // Turn on the ADC.
    counter=0;  
    // Set Interrupt
    ConfigIntADC10(ADC_INT_ON | ADC_INT_PRI_5);
}

void __ISR(_ADC_VECTOR, IPL5SOFT) ADCHandler(void)
{
    int tmp,j;  
    //PORTECLR = 0x01;
    //PORTEINV=0x01;
    // I had to remove the 16-bit pointer way to get access to the buffer
    // because for some reason the BUF1 was in memory 16 Bytes after BUF0.
    // I thought this should be 16 bits, and so pointer arithmatic using an
    // unsigned short int won't work.  I will have to look into this later.
    
    // For DFM V3, here I will attempt to load the proper values into CurrentValues such
    // that they go A1, A2, B1, B2, etc., with the last one being the
    // voltage value.
    tmp=ADC1BUF0; // Voltage
    CurrentValues[12]+=(tmp-values[12][counter]);
    values[12][counter]=tmp;

    tmp=ADC1BUF1; //A1    
    CurrentValues[0]+=(tmp-values[0][counter]);
    values[0][counter]=tmp;

    tmp=ADC1BUF2; //B1    
    CurrentValues[2]+=(tmp-values[2][counter]);
    values[2][counter]=tmp;

    tmp=ADC1BUF3; // A2    
    CurrentValues[1]+=(tmp-values[1][counter]);
    values[1][counter]=tmp;

    tmp=ADC1BUF4; // B2    
    CurrentValues[3]+=(tmp-values[3][counter]);
    values[3][counter]=tmp;

    tmp=ADC1BUF5; // F1    
    CurrentValues[10]+=(tmp-values[10][counter]);
    values[10][counter]=tmp;

    tmp=ADC1BUF6; // D2   
    CurrentValues[7]+=(tmp-values[7][counter]);
    values[7][counter]=tmp;

    tmp=ADC1BUF7; // F2   
    CurrentValues[11]+=(tmp-values[11][counter]);
    values[11][counter]=tmp;

    tmp=ADC1BUF8; // C1   
    CurrentValues[4]+=(tmp-values[4][counter]);
    values[4][counter]=tmp;

    tmp=ADC1BUF9; // E1    
    CurrentValues[8]+=(tmp-values[8][counter]);
    values[8][counter]=tmp;

    tmp=ADC1BUFA;  // C2  
    CurrentValues[5]+=(tmp-values[5][counter]);
    values[5][counter]=tmp;

    tmp=ADC1BUFB; // E2   
    CurrentValues[9]+=(tmp-values[9][counter]);
    values[9][counter]=tmp;

    tmp=ADC1BUFC; // D1   
    CurrentValues[6]+=(tmp-values[6][counter]);
    values[6][counter]=tmp;

    counter++; 
    if(counter>=128) {
          counter=0;
          //PORTEINV=0x01;
    }
           
    analogUpdateFlag=1;
    INTClearFlag(INT_AD1);
    //PORTESET = 0x01;
}
