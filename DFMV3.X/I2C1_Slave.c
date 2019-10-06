#include "GlobalIncludes.h"


// Normally I2C devices run at either 100khz or 400 khz, although some can do 2MHz
// The propellor seems to only manage 100khz
#define BAUDRATE 400000 // Set at 400000 for normal EEPROM and 100000 for Propeller

extern int volatile CurrentValues[13];
extern int volatile isInDarkMode;
extern unsigned char volatile OptoState1;
extern unsigned char volatile OptoState2 ;
extern unsigned char volatile pulseWidth_ms;
extern unsigned char volatile hertz;
int volatile WrittenValues[16];


void I2C2_Configure(void) {
    int brg, baud;
    baud = BAUDRATE;
    if ((GetPeripheralClock()) == 40000000) {
        if (BAUDRATE == 100000)
            brg = 0x0C6;
        if (BAUDRATE == 400000)
            brg = 0x030;
    }
    // Note that strict addressing is enabled. THis forbids certain device address from being sent
    // or responded to. Details in the PIC32 datasheet.
    OpenI2C2(I2C_ON | I2C_IDLE_CON | I2C_CLK_REL | I2C_STRICT_EN | I2C_7BIT_ADD | I2C_SLW_DIS |
            I2C_SM_DIS | I2C_GC_EN | I2C_STR_EN | I2C_ACK | I2C_ACK_DIS | I2C_RCV_DIS | I2C_STOP_DIS |
            I2C_RESTART_DIS | I2C_START_DIS, brg);

    //OpenI2C2(I2C_EN,brg);

    I2C2ADD = BASESLAVEADDRESS + SPECIFIC_SLAVE_ID;
    I2C2MSK = 0x00; // No mask.  We care about all address bits.

    SetPriorityIntI2C2(4);
    EnableIntSI2C2;
    INTClearFlag(INT_I2C2S);
}

void I2C2Respond(void) {
    int dataRead, i;
    unsigned int tmp2;
    unsigned char tmp;
    static unsigned char command, argument;
    static unsigned char *pValue;
    static unsigned int dIndex;
    static unsigned int wIndex;
    if ((I2C2STATbits.R_W == 0) && (I2C2STATbits.D_A == 0)) {
        // R/W bit = 0 --> indicates data transfer is input to slave
        // D/A bit = 1 --> indicates last byte was data

        // So this means the Master sent a device address and write request
        // It should follow with a write request and data.

        // read the address data from the buffer to avoid
        // subsequent overflow.             
        dataRead = SlaveReadI2C2();
        wIndex = 0;
        // release the clock to restart I2C
        I2C2CONbits.SCLREL = 1; // release clock stretch bit
    } else if ((I2C2STATbits.R_W == 0) && (I2C2STATbits.D_A == 1)) {
        // This means the Master sent a data and write request
        if (wIndex == 0) {
            // Because wIndex is reset when the address is sent
            // then this is the first byte...the command.
            command = SlaveReadI2C2();
            wIndex++;
        } else if (wIndex==1){
            argument = SlaveReadI2C2();
            switch (command) {
                case 0x01:
                    if (argument == 0x00)
                        isInDarkMode = 0;
                    else if (argument == 0x01) {
                        isInDarkMode = 1;
                        HEARTBEAT_LAT = 0;
                        SIGNAL_LED_OFF();
                    }
                    break;
                case 0x02: // Need to expand this to be specific for 6 chamber setup.
                    OptoState1 = argument;
                    break;
                case 0x03:
                    break; // This is to be implemented 12-well setup.
                case 0x04:
                    SetHertz(argument);
                    break;
                case 0x05:
                    SetPulseWidth_ms(argument);
                    break;
            }
            wIndex++;
        }
        else if(wIndex==2){
            argument = SlaveReadI2C2();
            switch (command) {
                case 0x02:
                    OptoState2=argument;
                    break;            
            }
        }
        else {
            
        }
        // release the clock to restart I2C
        I2C2CONbits.SCLREL = 1; // release clock stretch bit
    } else if ((I2C2STATbits.R_W == 1) && (I2C2STATbits.D_A == 0)) {
        //if (IsInDarkMode == 0)
        //    LED1_ON();
        // So this means the Master sent an address and read request
        dataRead = SlaveReadI2C2(); // This just empties the receive buffer.
        // We need to freeze the current values at the point when the MCU
        // calls for them because the analog interrupt may change them.
        WrittenValues[15] = 0;
        for (i = 0; i < 12; i++) {
            WrittenValues[i] = CurrentValues[i];
            WrittenValues[15] += WrittenValues[i];
        }
        tmp2=isInDarkMode + (hertz << 8) + (pulseWidth_ms << 16);
        WrittenValues[12] = tmp2;        
        WrittenValues[13] = OptoState1;      
        WrittenValues[14] = OptoState2;
        WrittenValues[15] += tmp2+OptoState1+OptoState2;
        pValue = (unsigned char *) WrittenValues;
        SlaveWriteI2C2(*pValue);
        //SlaveWriteI2C2(0);
        pValue++;
        dIndex = 1;
        if (isInDarkMode == 0) SIGNAL_LED_TOGGLE();
    } else if ((I2C2STATbits.R_W == 1) && (I2C2STATbits.D_A == 1)) {
        // So this means the Master sent a data and read request

        // Send twelve data bytes and one checksum byte
        if (dIndex >= 64)
            SlaveWriteI2C2(0);
        else {
            SlaveWriteI2C2(*pValue);
            //SlaveWriteI2C2(0);
            pValue++;
            dIndex++;
        }
        I2C2CONbits.SCLREL = 1; // release clock stretch bit
        ; // After writing current values, update the next ones.
    }
}

void __ISR(_I2C_2_VECTOR, IPL4AUTO) I2C2InterruptServiceRoutine(void) {
    if (IFS1bits.I2C2MIF == 1) { // Should never be here because we don't operate as slave.
        //LED1_ON();
        mI2C2MClearIntFlag();
        return;
    }
    if (IFS1bits.I2C2BIF == 1) {
        //LED1_ON();
        mI2C2BClearIntFlag();
        return;
    }
    // If not the previous two, then I2C1SIF is thrown, which corresponds to a slave interrupt
    I2C2Respond();
    INTClearFlag(INT_I2C2S);
}


