/*********************************************************************
 *
 *	UNO32 board specific hardware definitions
 *
 *	UNO32 has PIC32MX320F128
 *********************************************************************
 *
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Scott Pletcher       02/16/13    First code for UNO32.
 ********************************************************************/
#ifndef __HARDWARE_PROFILE_H
#define __HARDWARE_PROFILE_H

#include <p32xxxx.h>

/*********************************************************************
* Sets Hearbeat values
* This value will be inverted each click
********************************************************************/

#define HEARTBEAT_LAT  _LATE3
#define HEARTBEAT_PORT  _RE3
#define HEARTBEAT_TRIS  _TRISE3
#define HEARTBEAT_LED_TOGGLE() PORTEINV = 0x0008
#define HEARTBEAT_LED_OFF() PORTESET =0x0008
#define HEARTBEAT_LED_ON() PORTECLR =0x0008

#define SIGNAL_LED_LAT _LATE2
#define SIGNAL_LED_PORT _RE2
#define SIGNAL_LED_TRIS _TRISE2
#define FLIP_SIGNAL_LED() PORTEINV = 0x0004
#define SIGNAL_LED_OFF() PORTESET =0x0004
#define SIGNAL_LED_ON() PORTECLR =0x0004
#define SIGNAL_LED_TOGGLE() PORTEINV =0x0004

#define BLUE_LED_LAT _LATE1
#define BLUE_LED_PORT _RE1
#define BLUE_LED_TRIS _TRISE1
#define BLUE_SIGNAL_LED() PORTEINV = 0x0002
#define BLUE_LED_OFF() PORTESET =0x0002
#define BLUE_LED_ON() PORTECLR =0x0002

#define YELLOW_LED_LAT _LATE0
#define YELLOW_LED_PORT _RE0
#define YELLOW_LED_TRIS _TRISE0
#define YELLOW_LED() PORTEINV = 0x0001
#define YELLOW_LED_OFF() PORTESET =0x0001
#define YELLOW_LED_ON() PORTECLR =0x0001

#define TIMER_PIN_LAT _LATE7
#define TIMER_PIN_TRIS _TRISE7
#define TIMER_PIN_OFF() PORTECLR =0x0080
#define TIMER_PIN_ON() PORTESET =0x0080
#define TIMER_PIN_TOGGLE() PORTEINV =0x0080

/*********************************************************************
* GetSystemClock() returns system clock frequency.
*
* GetPeripheralClock() returns peripheral clock frequency.
*
********************************************************************/

#define GetSystemClock() 			(80000000ul)

/*********************************************************************
* Macro: #define	GetPeripheralClock() 
*
* Overview: This macro returns the peripheral clock frequency 
*			used in Hertz.
*			* value for PIC32 is <PRE>(GetSystemClock()/(1<<OSCCONbits.PBDIV)) </PRE>
*
* GetInstructionClock() is required for the new Microchip Libraries 2010-10-19
********************************************************************/
#define	GetPeripheralClock()		(GetSystemClock()/(1<<OSCCONbits.PBDIV))
#define GetInstructionClock() (GetSystemClock())
/*********************************************************************/



#define SWITCH_TRIS _TRISE4
#define SWITCH_PORT _RE4

#define BUTTON1_TRIS _TRISE6
#define BUTTON1_PORT _RE6

#define BUTTON2_TRIS _TRISE5
#define BUTTON2_PORT _RE5

#define ID_SWITCH1_TRIS _TRISD12
#define IDSWITCH1_PORT _RD12

#define ID_SWITCH2_TRIS _TRISD13
#define IDSWITCH2_PORT _RD13

#define ID_SWITCH4_TRIS _TRISD14
#define IDSWITCH4_PORT _RD14

#define ID_SWITCH8_TRIS _TRISD15
#define IDSWITCH8_PORT _RD15



#endif // __HARDWARE_PROFILE_H
