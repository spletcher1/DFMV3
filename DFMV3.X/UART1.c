#include "GlobalIncludes.h"

#define UART3_ACK               0xFF
// How many times to send and wait for ack before returning
#define NUMBER_RETRIES_FOR_ACK  2

// Basic Packet Bytes
#define UART3_START_COMMAND     0xF1
#define UART3_END_COMMAND       0xFA
#define UART3_ACK               0xFF
#define UART3_CHKSUM_ERROR      0xFE
#define UART3_STRING_DELIM      0x00
#define NEGATIVE_ONE            0xFD

#define UART3_SINGLE_COMMAND    0x10
#define UART3_BINARY_COMMAND    0x13
#define UART3_TRINARY_COMMAND   0x14

typedef enum {
    SUCCESS,
    NO_REPLY,
    PORT_CLOSED,
    CHKSUM_ERROR
} RESPONSECODES;

#define UART1BUFFERSIZE 200
unsigned char buffer[UART1BUFFERSIZE];
unsigned int buffercounter;
BYTE CommandReceived;
BYTE AckFound;
BYTE ChkSumErrorFound;

inline void ClearUART3Buffer() {
    buffercounter = 0;
}

inline void SendAck() {
    while (!UARTTransmitterIsReady(UART3));
    UARTSendDataByte(UART3, UART3_ACK);
}

/////////////////////////////////////////////////////////////////
// Select one to specify configuration.
// Baud Rate 19200
// Baud rate 38400
// Baud rate 115200
#define TARGET_BAUD_RATE  115200

/////////////////////////////////////////////////////////////////

void ConfigureDefaultUART3(void) {
    // Note: As of now, the baud rate set for the parallax RFID reader is 2400.
    // Data bits = 8; no parity; stop bits = 1;

    UARTConfigure(UART3, UART_ENABLE_HIGH_SPEED);
    UARTSetFifoMode(UART3, UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART3, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART3, GetPeripheralClock(), TARGET_BAUD_RATE);
    UARTEnable(UART3, UART_ENABLE_FLAGS(UART_ENABLE | UART_TX | UART_PERIPHERAL));

    // Interrupt-specific code below.
    ClearUART3Buffer();

    AckFound = 0;
}

void outbyte(char c){
        while (!UARTTransmitterIsReady(UART3));
        UARTSendDataByte(UART3, c);
}


void ShortIntToUART3(int a){
  unsigned char c;
  c = a>>8;
  while (!UARTTransmitterIsReady(UART3));
       UARTSendDataByte(UART3, c);
  c = a;
  while (!UARTTransmitterIsReady(UART3));
       UARTSendDataByte(UART3, c); 
}

void IntToUART3(int a){
  unsigned char c;
  c = a;
  while (!UARTTransmitterIsReady(UART3));
       UARTSendDataByte(UART3, c);
  c = a>>8;
  while (!UARTTransmitterIsReady(UART3));
       UARTSendDataByte(UART3, c);
  c = a>>16;
  while (!UARTTransmitterIsReady(UART3));
       UARTSendDataByte(UART3, c);
  c = a>>24;
  while (!UARTTransmitterIsReady(UART3));
       UARTSendDataByte(UART3, c);
}

void StringToUART3(const char* buffer) {
    while (*buffer != '\0') {
        while (!UARTTransmitterIsReady(UART3));
        UARTSendDataByte(UART3, *buffer++);
    }
}

