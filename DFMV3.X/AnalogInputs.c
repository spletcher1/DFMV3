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
    AD1CON2bits.VCFG = 0x03;  // Use external VRef+ and minus pins
    Nop();Nop();
    // AD1CHSbits.CH0SA = 0x03; // No need to link to MUX A because we will autosample.
    Nop();Nop();
    AD1CON2bits.CSCNA = 1; // Enable input scan,
     Nop();Nop();
   
    // Need to make the buffer be composed of one 16 bit buffer.  This will
    // allow me to update the contents in one buffer while readings are still being loaded into the second.
    AD1CON2bits.BUFM = 0;

    Nop();Nop();
    // Note that SMPI should be one less than the actual number of conversions required (see data sheet).
    // SMPI=0x00 means interrupt after each conversion.
    // This is especially important for PIC5600/600/700 which won't clear the interrupt
    // unles all buffers are read.
    AD1CON2bits.SMPI = 0x0C; // Make the buffer thirteen entries (12 wells plus the volts in input).
    Nop();Nop();
    AD1CON2bits.ALTS = 0; // Always sample from MUX A (do not alternate between A and B).
    Nop();Nop();

    // The following analog inputs are used
    // AIIndex[12] = {0,2,3,4,5,8,9,10,11,12,13,14,15};
    // Set the appropriate bits in AD1CSSL
    AD1CSSL =0xFF3D;

    AD1CON3bits.ADRC=0; // Use peripheral bus as ADC clock (won't work in sleep mode).
    Nop();Nop();
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

    AD1CON3bits.SAMC = 0x1F;
    Nop();Nop();  
    AD1CON3bits.ADCS = 0x4A;  
     Nop();Nop();   
}

void StartContinuousSampling(){
    ClearAnalogValues();
    ConfigureScanningAnalogInputs();
    AD1CON1SET = 0x0004; // Set auto.
    AD1CON1SET = 0x8000; // Turn on the ADC.
    counter=0;  
    // Set Interrupt
    ConfigIntADC10(ADC_INT_ON | ADC_INT_PRI_4);
}

// If there are problems will this interrupt maybe it is because it is sticky?
// Yes, the ADC interrupt is only sticky (the word used in the datasheets is actually 
// persistent) in PIC32MX5XX/6XX/7XX. See table A-1 in APPENDIX A in the 
// PIC32MX5XX/6XX/7XX Family Data Sheet. (https://www.microchip.com/forums/m745323.aspx)

// Again, summarizing above, this interrupt is called about every 2.1ms.
// It takes about 4us to execute the interrupt.

void __ISR(_ADC_VECTOR, IPL4SOFT) ADCHandler(void)
{
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
    INTClearFlag(INT_AD1);
    //PORTECLR = 0x01;
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