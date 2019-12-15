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


#define HEARTBEAT_LAT  _RE3
#define HEARTBEAT_TRIS  _TRISE3
#define FLIP_HEARTBEAT_LED() PORTEINV = 0x0008


#define GREENLED_LAT  _RE2
#define GREENLED_TRIS  _TRISE2
#define FLIP_GREEN_LED() PORTEINV = 0x0004

#define BLUELED_LAT  _RE1
#define BLUELED_TRIS  _TRISE1
#define FLIP_BLUE_LED() PORTEINV = 0x0002

#define YELLOWLED_LAT  _RE0
#define YELLOWLED_TRIS  _TRISE0
#define FLIP_YELLOW_LED() PORTEINV = 0x0001

#define SWITCH_TRIS _TRISE4
#define SWITCH_PORT _RE4

#define USERBUTTON1_TRIS _TRISE6
#define USERBUTTON1_PORT _RE6

#define USERBUTTON2_TRIS _TRISE5
#define USERBUTTON2_PORT _RE5

#define BUTTON1_DEBOUNCE_COUNTS 100
#define BUTTON2_DEBOUNCE_COUNTS 100

#define IDSELECTOR1_TRIS _TRISD12
#define IDSELECTOR1_PORT _RD12
#define IDSELECTOR2_TRIS _TRISD13
#define IDSELECTOR2_PORT _RD13
#define IDSELECTOR4_TRIS _TRISD14
#define IDSELECTOR4_PORT _RD14
#define IDSELECTOR8_TRIS _TRISD15
#define IDSELECTOR8_PORT _RD15
#define SETIDSELECTOR_TRIS() TRISDSET = 0x0000F000
#define GETIDSELECTOR_VALUE() ((~PORTD & 0xF000) >> 12)

#define RX485_ENABLE_SEND() _LATF13=1
#define RX485_DISABLE_SEND() _LATF13=0

#define RX485_SEND_TRIS _TRISF13


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





#endif // __HARDWARE_PROFILE_H
