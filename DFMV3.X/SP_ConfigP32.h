#ifndef __SP_CONFIG_H
#define __SP_CONFIG_H

// General Configuration
// This file has been optimized for the PIC32 LV32MX Board from Mikroelectronika.

#pragma config FWDTEN = OFF // No watchdog timer
#pragma config WDTPS = PS1 // Watchdog postscaler if enabled
#pragma config FCKSM = CSDCMD // Clock switching and monitoring disabled
#pragma config OSCIOFNC = OFF // Clock output on OSC0?
#pragma config IESO = OFF // Internal external switch over bit
#pragma config FSOSCEN = OFF // Secondary oscillator enable bit

#pragma config CP=OFF // Code protect bit
#pragma config BWP = OFF // Boot flash write protect
#pragma config PWP = OFF // Program flash write protect
#pragma config ICESEL = ICS_PGx2 // ICE pins are shared with PGC1, PGD1 on the UNO32 Stick
#pragma config DEBUG = OFF // Background debugger


// ************ OSCILLATOR SETUP *******************
// This assumes an 8MHz crystal attached to the primary oscillator
// pins OSC1 and OSC0. 
#pragma config POSCMOD=XT  // Low speed external oscillator (assume 8MHz)
#pragma config FNOSC=PRIPLL // Oscillator selector (PRIPLL = primary oscillator w/PLL)
#pragma config FPLLODIV=DIV_1 //PLL output divider
#pragma config FPLLIDIV=DIV_2  //PLL input divider (Assume 8MHz)
#pragma config FPLLMUL=MUL_20 // PLL Multiplier
#pragma config FPBDIV = DIV_2 // Bootup PBCLK divider (peripheral divider)

// Standard setup for 80MHz function from 8MHz crystal:
// 8MHz -> PLL Divider (2) -> PLL Multiplier (20) -> Output Divider (1) -> 80MHz
// Will use a standard peripheral bus of 40MHz

#endif
