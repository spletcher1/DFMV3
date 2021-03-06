/*******************************************************************************
  GPIO PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_gpio.h

  Summary:
    GPIO PLIB Header File

  Description:
    This library provides an interface to control and interact with Parallel
    Input/Output controller (GPIO) module.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#ifndef PLIB_GPIO_H
#define PLIB_GPIO_H

#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************


/*** Macros for GPIO_RG15 pin ***/
#define GPIO_RG15_Set()               (LATGSET = (1<<15))
#define GPIO_RG15_Clear()             (LATGCLR = (1<<15))
#define GPIO_RG15_Toggle()            (LATGINV= (1<<15))
#define GPIO_RG15_OutputEnable()      (TRISGCLR = (1<<15))
#define GPIO_RG15_InputEnable()       (TRISGSET = (1<<15))
#define GPIO_RG15_Get()               ((PORTG >> 15) & 0x1)
#define GPIO_RG15_PIN                  GPIO_PIN_RG15

/*** Macros for GPIO_RE5 pin ***/
#define GPIO_RE5_Set()               (LATESET = (1<<5))
#define GPIO_RE5_Clear()             (LATECLR = (1<<5))
#define GPIO_RE5_Toggle()            (LATEINV= (1<<5))
#define GPIO_RE5_OutputEnable()      (TRISECLR = (1<<5))
#define GPIO_RE5_InputEnable()       (TRISESET = (1<<5))
#define GPIO_RE5_Get()               ((PORTE >> 5) & 0x1)
#define GPIO_RE5_PIN                  GPIO_PIN_RE5

/*** Macros for GPIO_RE6 pin ***/
#define GPIO_RE6_Set()               (LATESET = (1<<6))
#define GPIO_RE6_Clear()             (LATECLR = (1<<6))
#define GPIO_RE6_Toggle()            (LATEINV= (1<<6))
#define GPIO_RE6_OutputEnable()      (TRISECLR = (1<<6))
#define GPIO_RE6_InputEnable()       (TRISESET = (1<<6))
#define GPIO_RE6_Get()               ((PORTE >> 6) & 0x1)
#define GPIO_RE6_PIN                  GPIO_PIN_RE6

/*** Macros for GPIO_RG6 pin ***/
#define GPIO_RG6_Set()               (LATGSET = (1<<6))
#define GPIO_RG6_Clear()             (LATGCLR = (1<<6))
#define GPIO_RG6_Toggle()            (LATGINV= (1<<6))
#define GPIO_RG6_OutputEnable()      (TRISGCLR = (1<<6))
#define GPIO_RG6_InputEnable()       (TRISGSET = (1<<6))
#define GPIO_RG6_Get()               ((PORTG >> 6) & 0x1)
#define GPIO_RG6_PIN                  GPIO_PIN_RG6

/*** Macros for GPIO_RG7 pin ***/
#define GPIO_RG7_Set()               (LATGSET = (1<<7))
#define GPIO_RG7_Clear()             (LATGCLR = (1<<7))
#define GPIO_RG7_Toggle()            (LATGINV= (1<<7))
#define GPIO_RG7_OutputEnable()      (TRISGCLR = (1<<7))
#define GPIO_RG7_InputEnable()       (TRISGSET = (1<<7))
#define GPIO_RG7_Get()               ((PORTG >> 7) & 0x1)
#define GPIO_RG7_PIN                  GPIO_PIN_RG7

/*** Macros for GPIO_RG8 pin ***/
#define GPIO_RG8_Set()               (LATGSET = (1<<8))
#define GPIO_RG8_Clear()             (LATGCLR = (1<<8))
#define GPIO_RG8_Toggle()            (LATGINV= (1<<8))
#define GPIO_RG8_OutputEnable()      (TRISGCLR = (1<<8))
#define GPIO_RG8_InputEnable()       (TRISGSET = (1<<8))
#define GPIO_RG8_Get()               ((PORTG >> 8) & 0x1)
#define GPIO_RG8_PIN                  GPIO_PIN_RG8

/*** Macros for GPIO_RF13 pin ***/
#define GPIO_RF13_Set()               (LATFSET = (1<<13))
#define GPIO_RF13_Clear()             (LATFCLR = (1<<13))
#define GPIO_RF13_Toggle()            (LATFINV= (1<<13))
#define GPIO_RF13_OutputEnable()      (TRISFCLR = (1<<13))
#define GPIO_RF13_InputEnable()       (TRISFSET = (1<<13))
#define GPIO_RF13_Get()               ((PORTF >> 13) & 0x1)
#define GPIO_RF13_PIN                  GPIO_PIN_RF13

/*** Macros for GPIO_RD14 pin ***/
#define GPIO_RD14_Set()               (LATDSET = (1<<14))
#define GPIO_RD14_Clear()             (LATDCLR = (1<<14))
#define GPIO_RD14_Toggle()            (LATDINV= (1<<14))
#define GPIO_RD14_OutputEnable()      (TRISDCLR = (1<<14))
#define GPIO_RD14_InputEnable()       (TRISDSET = (1<<14))
#define GPIO_RD14_Get()               ((PORTD >> 14) & 0x1)
#define GPIO_RD14_PIN                  GPIO_PIN_RD14

/*** Macros for GPIO_RD15 pin ***/
#define GPIO_RD15_Set()               (LATDSET = (1<<15))
#define GPIO_RD15_Clear()             (LATDCLR = (1<<15))
#define GPIO_RD15_Toggle()            (LATDINV= (1<<15))
#define GPIO_RD15_OutputEnable()      (TRISDCLR = (1<<15))
#define GPIO_RD15_InputEnable()       (TRISDSET = (1<<15))
#define GPIO_RD15_Get()               ((PORTD >> 15) & 0x1)
#define GPIO_RD15_PIN                  GPIO_PIN_RD15

/*** Macros for GPIO_RD8 pin ***/
#define GPIO_RD8_Set()               (LATDSET = (1<<8))
#define GPIO_RD8_Clear()             (LATDCLR = (1<<8))
#define GPIO_RD8_Toggle()            (LATDINV= (1<<8))
#define GPIO_RD8_OutputEnable()      (TRISDCLR = (1<<8))
#define GPIO_RD8_InputEnable()       (TRISDSET = (1<<8))
#define GPIO_RD8_Get()               ((PORTD >> 8) & 0x1)
#define GPIO_RD8_PIN                  GPIO_PIN_RD8

/*** Macros for GPIO_RD9 pin ***/
#define GPIO_RD9_Set()               (LATDSET = (1<<9))
#define GPIO_RD9_Clear()             (LATDCLR = (1<<9))
#define GPIO_RD9_Toggle()            (LATDINV= (1<<9))
#define GPIO_RD9_OutputEnable()      (TRISDCLR = (1<<9))
#define GPIO_RD9_InputEnable()       (TRISDSET = (1<<9))
#define GPIO_RD9_Get()               ((PORTD >> 9) & 0x1)
#define GPIO_RD9_PIN                  GPIO_PIN_RD9

/*** Macros for GPIO_RD10 pin ***/
#define GPIO_RD10_Set()               (LATDSET = (1<<10))
#define GPIO_RD10_Clear()             (LATDCLR = (1<<10))
#define GPIO_RD10_Toggle()            (LATDINV= (1<<10))
#define GPIO_RD10_OutputEnable()      (TRISDCLR = (1<<10))
#define GPIO_RD10_InputEnable()       (TRISDSET = (1<<10))
#define GPIO_RD10_Get()               ((PORTD >> 10) & 0x1)
#define GPIO_RD10_PIN                  GPIO_PIN_RD10

/*** Macros for GPIO_RD11 pin ***/
#define GPIO_RD11_Set()               (LATDSET = (1<<11))
#define GPIO_RD11_Clear()             (LATDCLR = (1<<11))
#define GPIO_RD11_Toggle()            (LATDINV= (1<<11))
#define GPIO_RD11_OutputEnable()      (TRISDCLR = (1<<11))
#define GPIO_RD11_InputEnable()       (TRISDSET = (1<<11))
#define GPIO_RD11_Get()               ((PORTD >> 11) & 0x1)
#define GPIO_RD11_PIN                  GPIO_PIN_RD11

/*** Macros for GPIO_RD0 pin ***/
#define GPIO_RD0_Set()               (LATDSET = (1<<0))
#define GPIO_RD0_Clear()             (LATDCLR = (1<<0))
#define GPIO_RD0_Toggle()            (LATDINV= (1<<0))
#define GPIO_RD0_OutputEnable()      (TRISDCLR = (1<<0))
#define GPIO_RD0_InputEnable()       (TRISDSET = (1<<0))
#define GPIO_RD0_Get()               ((PORTD >> 0) & 0x1)
#define GPIO_RD0_PIN                  GPIO_PIN_RD0

/*** Macros for GPIO_RD1 pin ***/
#define GPIO_RD1_Set()               (LATDSET = (1<<1))
#define GPIO_RD1_Clear()             (LATDCLR = (1<<1))
#define GPIO_RD1_Toggle()            (LATDINV= (1<<1))
#define GPIO_RD1_OutputEnable()      (TRISDCLR = (1<<1))
#define GPIO_RD1_InputEnable()       (TRISDSET = (1<<1))
#define GPIO_RD1_Get()               ((PORTD >> 1) & 0x1)
#define GPIO_RD1_PIN                  GPIO_PIN_RD1

/*** Macros for GPIO_RD2 pin ***/
#define GPIO_RD2_Set()               (LATDSET = (1<<2))
#define GPIO_RD2_Clear()             (LATDCLR = (1<<2))
#define GPIO_RD2_Toggle()            (LATDINV= (1<<2))
#define GPIO_RD2_OutputEnable()      (TRISDCLR = (1<<2))
#define GPIO_RD2_InputEnable()       (TRISDSET = (1<<2))
#define GPIO_RD2_Get()               ((PORTD >> 2) & 0x1)
#define GPIO_RD2_PIN                  GPIO_PIN_RD2

/*** Macros for GPIO_RD3 pin ***/
#define GPIO_RD3_Set()               (LATDSET = (1<<3))
#define GPIO_RD3_Clear()             (LATDCLR = (1<<3))
#define GPIO_RD3_Toggle()            (LATDINV= (1<<3))
#define GPIO_RD3_OutputEnable()      (TRISDCLR = (1<<3))
#define GPIO_RD3_InputEnable()       (TRISDSET = (1<<3))
#define GPIO_RD3_Get()               ((PORTD >> 3) & 0x1)
#define GPIO_RD3_PIN                  GPIO_PIN_RD3

/*** Macros for GPIO_RD12 pin ***/
#define GPIO_RD12_Set()               (LATDSET = (1<<12))
#define GPIO_RD12_Clear()             (LATDCLR = (1<<12))
#define GPIO_RD12_Toggle()            (LATDINV= (1<<12))
#define GPIO_RD12_OutputEnable()      (TRISDCLR = (1<<12))
#define GPIO_RD12_InputEnable()       (TRISDSET = (1<<12))
#define GPIO_RD12_Get()               ((PORTD >> 12) & 0x1)
#define GPIO_RD12_PIN                  GPIO_PIN_RD12

/*** Macros for GPIO_RD13 pin ***/
#define GPIO_RD13_Set()               (LATDSET = (1<<13))
#define GPIO_RD13_Clear()             (LATDCLR = (1<<13))
#define GPIO_RD13_Toggle()            (LATDINV= (1<<13))
#define GPIO_RD13_OutputEnable()      (TRISDCLR = (1<<13))
#define GPIO_RD13_InputEnable()       (TRISDSET = (1<<13))
#define GPIO_RD13_Get()               ((PORTD >> 13) & 0x1)
#define GPIO_RD13_PIN                  GPIO_PIN_RD13

/*** Macros for GPIO_RD4 pin ***/
#define GPIO_RD4_Set()               (LATDSET = (1<<4))
#define GPIO_RD4_Clear()             (LATDCLR = (1<<4))
#define GPIO_RD4_Toggle()            (LATDINV= (1<<4))
#define GPIO_RD4_OutputEnable()      (TRISDCLR = (1<<4))
#define GPIO_RD4_InputEnable()       (TRISDSET = (1<<4))
#define GPIO_RD4_Get()               ((PORTD >> 4) & 0x1)
#define GPIO_RD4_PIN                  GPIO_PIN_RD4

/*** Macros for GPIO_RD5 pin ***/
#define GPIO_RD5_Set()               (LATDSET = (1<<5))
#define GPIO_RD5_Clear()             (LATDCLR = (1<<5))
#define GPIO_RD5_Toggle()            (LATDINV= (1<<5))
#define GPIO_RD5_OutputEnable()      (TRISDCLR = (1<<5))
#define GPIO_RD5_InputEnable()       (TRISDSET = (1<<5))
#define GPIO_RD5_Get()               ((PORTD >> 5) & 0x1)
#define GPIO_RD5_PIN                  GPIO_PIN_RD5

/*** Macros for GPIO_RD6 pin ***/
#define GPIO_RD6_Set()               (LATDSET = (1<<6))
#define GPIO_RD6_Clear()             (LATDCLR = (1<<6))
#define GPIO_RD6_Toggle()            (LATDINV= (1<<6))
#define GPIO_RD6_OutputEnable()      (TRISDCLR = (1<<6))
#define GPIO_RD6_InputEnable()       (TRISDSET = (1<<6))
#define GPIO_RD6_Get()               ((PORTD >> 6) & 0x1)
#define GPIO_RD6_PIN                  GPIO_PIN_RD6

/*** Macros for GPIO_RD7 pin ***/
#define GPIO_RD7_Set()               (LATDSET = (1<<7))
#define GPIO_RD7_Clear()             (LATDCLR = (1<<7))
#define GPIO_RD7_Toggle()            (LATDINV= (1<<7))
#define GPIO_RD7_OutputEnable()      (TRISDCLR = (1<<7))
#define GPIO_RD7_InputEnable()       (TRISDSET = (1<<7))
#define GPIO_RD7_Get()               ((PORTD >> 7) & 0x1)
#define GPIO_RD7_PIN                  GPIO_PIN_RD7

/*** Macros for GPIO_RE0 pin ***/
#define GPIO_RE0_Set()               (LATESET = (1<<0))
#define GPIO_RE0_Clear()             (LATECLR = (1<<0))
#define GPIO_RE0_Toggle()            (LATEINV= (1<<0))
#define GPIO_RE0_OutputEnable()      (TRISECLR = (1<<0))
#define GPIO_RE0_InputEnable()       (TRISESET = (1<<0))
#define GPIO_RE0_Get()               ((PORTE >> 0) & 0x1)
#define GPIO_RE0_PIN                  GPIO_PIN_RE0

/*** Macros for GPIO_RE1 pin ***/
#define GPIO_RE1_Set()               (LATESET = (1<<1))
#define GPIO_RE1_Clear()             (LATECLR = (1<<1))
#define GPIO_RE1_Toggle()            (LATEINV= (1<<1))
#define GPIO_RE1_OutputEnable()      (TRISECLR = (1<<1))
#define GPIO_RE1_InputEnable()       (TRISESET = (1<<1))
#define GPIO_RE1_Get()               ((PORTE >> 1) & 0x1)
#define GPIO_RE1_PIN                  GPIO_PIN_RE1

/*** Macros for GPIO_RE2 pin ***/
#define GPIO_RE2_Set()               (LATESET = (1<<2))
#define GPIO_RE2_Clear()             (LATECLR = (1<<2))
#define GPIO_RE2_Toggle()            (LATEINV= (1<<2))
#define GPIO_RE2_OutputEnable()      (TRISECLR = (1<<2))
#define GPIO_RE2_InputEnable()       (TRISESET = (1<<2))
#define GPIO_RE2_Get()               ((PORTE >> 2) & 0x1)
#define GPIO_RE2_PIN                  GPIO_PIN_RE2

/*** Macros for GPIO_RE3 pin ***/
#define GPIO_RE3_Set()               (LATESET = (1<<3))
#define GPIO_RE3_Clear()             (LATECLR = (1<<3))
#define GPIO_RE3_Toggle()            (LATEINV= (1<<3))
#define GPIO_RE3_OutputEnable()      (TRISECLR = (1<<3))
#define GPIO_RE3_InputEnable()       (TRISESET = (1<<3))
#define GPIO_RE3_Get()               ((PORTE >> 3) & 0x1)
#define GPIO_RE3_PIN                  GPIO_PIN_RE3

/*** Macros for GPIO_RE4 pin ***/
#define GPIO_RE4_Set()               (LATESET = (1<<4))
#define GPIO_RE4_Clear()             (LATECLR = (1<<4))
#define GPIO_RE4_Toggle()            (LATEINV= (1<<4))
#define GPIO_RE4_OutputEnable()      (TRISECLR = (1<<4))
#define GPIO_RE4_InputEnable()       (TRISESET = (1<<4))
#define GPIO_RE4_Get()               ((PORTE >> 4) & 0x1)
#define GPIO_RE4_PIN                  GPIO_PIN_RE4


// *****************************************************************************
/* GPIO Port

  Summary:
    Identifies the available GPIO Ports.

  Description:
    This enumeration identifies the available GPIO Ports.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/

typedef enum
{
    GPIO_PORT_A = 0,
    GPIO_PORT_B = 1,
    GPIO_PORT_C = 2,
    GPIO_PORT_D = 3,
    GPIO_PORT_E = 4,
    GPIO_PORT_F = 5,
    GPIO_PORT_G = 6,
} GPIO_PORT;

// *****************************************************************************
/* GPIO Port Pins

  Summary:
    Identifies the available GPIO port pins.

  Description:
    This enumeration identifies the available GPIO port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/

typedef enum
{
    GPIO_PIN_RA0 = 0,
    GPIO_PIN_RA1 = 1,
    GPIO_PIN_RA2 = 2,
    GPIO_PIN_RA3 = 3,
    GPIO_PIN_RA4 = 4,
    GPIO_PIN_RA5 = 5,
    GPIO_PIN_RA6 = 6,
    GPIO_PIN_RA7 = 7,
    GPIO_PIN_RA9 = 9,
    GPIO_PIN_RA10 = 10,
    GPIO_PIN_RA14 = 14,
    GPIO_PIN_RA15 = 15,
    GPIO_PIN_RB0 = 16,
    GPIO_PIN_RB1 = 17,
    GPIO_PIN_RB2 = 18,
    GPIO_PIN_RB3 = 19,
    GPIO_PIN_RB4 = 20,
    GPIO_PIN_RB5 = 21,
    GPIO_PIN_RB6 = 22,
    GPIO_PIN_RB7 = 23,
    GPIO_PIN_RB8 = 24,
    GPIO_PIN_RB9 = 25,
    GPIO_PIN_RB10 = 26,
    GPIO_PIN_RB11 = 27,
    GPIO_PIN_RB12 = 28,
    GPIO_PIN_RB13 = 29,
    GPIO_PIN_RB14 = 30,
    GPIO_PIN_RB15 = 31,
    GPIO_PIN_RC1 = 33,
    GPIO_PIN_RC2 = 34,
    GPIO_PIN_RC3 = 35,
    GPIO_PIN_RC4 = 36,
    GPIO_PIN_RC12 = 44,
    GPIO_PIN_RC13 = 45,
    GPIO_PIN_RC14 = 46,
    GPIO_PIN_RC15 = 47,
    GPIO_PIN_RD0 = 48,
    GPIO_PIN_RD1 = 49,
    GPIO_PIN_RD2 = 50,
    GPIO_PIN_RD3 = 51,
    GPIO_PIN_RD4 = 52,
    GPIO_PIN_RD5 = 53,
    GPIO_PIN_RD6 = 54,
    GPIO_PIN_RD7 = 55,
    GPIO_PIN_RD8 = 56,
    GPIO_PIN_RD9 = 57,
    GPIO_PIN_RD10 = 58,
    GPIO_PIN_RD11 = 59,
    GPIO_PIN_RD12 = 60,
    GPIO_PIN_RD13 = 61,
    GPIO_PIN_RD14 = 62,
    GPIO_PIN_RD15 = 63,
    GPIO_PIN_RE0 = 64,
    GPIO_PIN_RE1 = 65,
    GPIO_PIN_RE2 = 66,
    GPIO_PIN_RE3 = 67,
    GPIO_PIN_RE4 = 68,
    GPIO_PIN_RE5 = 69,
    GPIO_PIN_RE6 = 70,
    GPIO_PIN_RE7 = 71,
    GPIO_PIN_RE8 = 72,
    GPIO_PIN_RE9 = 73,
    GPIO_PIN_RF0 = 80,
    GPIO_PIN_RF1 = 81,
    GPIO_PIN_RF2 = 82,
    GPIO_PIN_RF3 = 83,
    GPIO_PIN_RF4 = 84,
    GPIO_PIN_RF5 = 85,
    GPIO_PIN_RF8 = 88,
    GPIO_PIN_RF12 = 92,
    GPIO_PIN_RF13 = 93,
    GPIO_PIN_RG0 = 96,
    GPIO_PIN_RG1 = 97,
    GPIO_PIN_RG2 = 98,
    GPIO_PIN_RG3 = 99,
    GPIO_PIN_RG6 = 102,
    GPIO_PIN_RG7 = 103,
    GPIO_PIN_RG8 = 104,
    GPIO_PIN_RG9 = 105,
    GPIO_PIN_RG12 = 108,
    GPIO_PIN_RG13 = 109,
    GPIO_PIN_RG14 = 110,
    GPIO_PIN_RG15 = 111,

    /* This element should not be used in any of the GPIO APIs.
       It will be used by other modules or application to denote that none of the GPIO Pin is used */
    GPIO_PIN_NONE = -1

} GPIO_PIN;

typedef enum
{
  CN0_PIN = 1 << 0,
  CN1_PIN = 1 << 1,
  CN2_PIN = 1 << 2,
  CN3_PIN = 1 << 3,
  CN4_PIN = 1 << 4,
  CN5_PIN = 1 << 5,
  CN6_PIN = 1 << 6,
  CN7_PIN = 1 << 7,
  CN8_PIN = 1 << 8,
  CN9_PIN = 1 << 9,
  CN10_PIN = 1 << 10,
  CN11_PIN = 1 << 11,
  CN12_PIN = 1 << 12,
  CN13_PIN = 1 << 13,
  CN14_PIN = 1 << 14,
  CN15_PIN = 1 << 15,
  CN16_PIN = 1 << 16,
  CN17_PIN = 1 << 17,
  CN18_PIN = 1 << 18,
  CN19_PIN = 1 << 19,
  CN20_PIN = 1 << 20,
  CN21_PIN = 1 << 21,
}CN_PIN;


void GPIO_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

uint32_t GPIO_PortRead(GPIO_PORT port);

void GPIO_PortWrite(GPIO_PORT port, uint32_t mask, uint32_t value);

uint32_t GPIO_PortLatchRead ( GPIO_PORT port );

void GPIO_PortSet(GPIO_PORT port, uint32_t mask);

void GPIO_PortClear(GPIO_PORT port, uint32_t mask);

void GPIO_PortToggle(GPIO_PORT port, uint32_t mask);

void GPIO_PortInputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortOutputEnable(GPIO_PORT port, uint32_t mask);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

static inline void GPIO_PinWrite(GPIO_PIN pin, bool value)
{
    GPIO_PortWrite((GPIO_PORT)(pin>>4), (uint32_t)(0x1) << (pin & 0xF), (uint32_t)(value) << (pin & 0xF));
}

static inline bool GPIO_PinRead(GPIO_PIN pin)
{
    return (bool)(((GPIO_PortRead((GPIO_PORT)(pin>>4))) >> (pin & 0xF)) & 0x1);
}

static inline bool GPIO_PinLatchRead(GPIO_PIN pin)
{
    return (bool)((GPIO_PortLatchRead((GPIO_PORT)(pin>>4)) >> (pin & 0xF)) & 0x1);
}

static inline void GPIO_PinToggle(GPIO_PIN pin)
{
    GPIO_PortToggle((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinSet(GPIO_PIN pin)
{
    GPIO_PortSet((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinClear(GPIO_PIN pin)
{
    GPIO_PortClear((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinInputEnable(GPIO_PIN pin)
{
    GPIO_PortInputEnable((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinOutputEnable(GPIO_PIN pin)
{
    GPIO_PortOutputEnable((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // PLIB_GPIO_H
