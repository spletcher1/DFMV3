#include "GlobalIncludes.h"

/**********************************************
 * On the Uno32 there is a very confusing renaming
 * of analog inputs.  They are translated here
 * First the AN header board
 * A0 = RB2 = ADC2
 * A1 = RB4 = ADC4
 * A2 = RB8 = ADC8
 * A3 = RB10 = ADC10
 * A4 = RB12 = ADC12
 * A5 = RB14 = ADC14
 * A6 = RB3 = ADC3
 * A7 = RB5 = ADC5
 * A8 = RB9 = ADC9
 * A9 = RB11 = ADC11
 * A10 = RB13 = ADC13
 * A11 = RB15 = ADC15
 * Pin 42 = RB0 = ADC15
 * Pin 41 = RB1 = ADC15
 * We don't have access to Pins 6 and 7

 * For the first six chambers we will use left right
 * pins on the head to represent left right inputs.
 *
 */
int volatile counter;
int volatile values[12][128];
int volatile CurrentValues[12];

void ClearAnalogValues(){
    int i,j;
     for (j = 0; j < 12; j++) {
      CurrentValues[j]=0;
      for (i = 0; i < 128; i++)
          values[j][i] = 0;
    }
    CurrentValues[12]=0;    
}

void ConfigureScanningAnalogInputs(){
    // COnfigure all analog inputs as such
    AD1PCFG = 0x0000;
    TRISB = 0xFFFF;
   
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
    AD1CON2bits.SMPI = 0x0C; // Make the buffer twelve entries.
    Nop();Nop();
    AD1CON2bits.ALTS = 0; // Always sample from MUX A (do not alternate between A and B).
    Nop();Nop();

    // The following analog inputs are used
    // AIIndex[12] = {2,3,4,5,8,9,10,11,12,13,14,15};
    // Set the appropriate bits in AD1CSSL
    AD1CSSL =0xFF3C;

    AD1CON3bits.ADRC=0; // Use peripheral bus as ADC clock (won't work in sleep mode).
    Nop();Nop();
    // If we sample 12 analog inputs 400 times per second, then we will be reading
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
    // One conversion = 161.25us.  Twelve conversions = 1.935ms.
    // 500 sets of 12 conversions = 967ms.
    // For some reason, when timing the rate of the interrupt, it is called
    // about every 2.1ms, which is a bit less often then predicted.
    // I am not exactly sure why at this moment.

    // If each report is called 200ms, it will encompass roughly the past 100 measures.
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
    ConfigureScanningAnalogInputs();
    AD1CON1SET = 0x0004; // Set auto.
    AD1CON1SET = 0x8000; // Turn on the ADC.
    counter=0;  
    // Set Interrupt
    ConfigIntADC10(ADC_INT_ON | ADC_INT_PRI_4);
}

void __ISR(_ADC_VECTOR, IPL4SOFT) ADCHandler(void)
{
    int tmp,j;  
    //TIMER_PIN_OFF();    //TIMER_PIN_TOGGLE();
    // I had to remove the 16-bit pointer way to get access to the buffer
    // because for some reason the BUF1 was in memory 16 Bytes after BUF0.
    // I thought this should be 16 bits, and so pointer arithmatic using an
    // unsigned short int won't work.  I will have to look into this later.
    tmp=ADC1BUF0;
    CurrentValues[0]+=(tmp-values[0][counter]);
    values[0][counter]=tmp;

    tmp=ADC1BUF1;    
    CurrentValues[1]+=(tmp-values[1][counter]);
    values[1][counter]=tmp;

    tmp=ADC1BUF2;    
    CurrentValues[2]+=(tmp-values[2][counter]);
    values[2][counter]=tmp;

    tmp=ADC1BUF3;    
    CurrentValues[3]+=(tmp-values[3][counter]);
    values[3][counter]=tmp;

    tmp=ADC1BUF4;    
    CurrentValues[4]+=(tmp-values[4][counter]);
    values[4][counter]=tmp;

    tmp=ADC1BUF5;    
    CurrentValues[5]+=(tmp-values[5][counter]);
    values[5][counter]=tmp;

    tmp=ADC1BUF6;    
    CurrentValues[6]+=(tmp-values[6][counter]);
    values[6][counter]=tmp;

    tmp=ADC1BUF7;    
    CurrentValues[7]+=(tmp-values[7][counter]);
    values[7][counter]=tmp;

    tmp=ADC1BUF8;    
    CurrentValues[8]+=(tmp-values[8][counter]);
    values[8][counter]=tmp;

    tmp=ADC1BUF9;    
    CurrentValues[9]+=(tmp-values[9][counter]);
    values[9][counter]=tmp;

    tmp=ADC1BUFA;    
    CurrentValues[10]+=(tmp-values[10][counter]);
    values[10][counter]=tmp;

    tmp=ADC1BUFB;    
    CurrentValues[11]+=(tmp-values[11][counter]);
    values[11][counter]=tmp;

    counter++;
    if(counter>=128) {
          counter=0;
          //TIMER_PIN_TOGGLE();
    }
    INTClearFlag(INT_AD1);
    //TIMER_PIN_ON();
}
