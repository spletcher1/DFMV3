#ifndef PACKETMANAGER_H
#define	PACKETMANAGER_H

struct StatusPacket *GetNextStatusInLine();
void StepPacketManager();
void InitializeStatusPacketBuffer();

struct StatusPacket {
  unsigned char ID;  
  unsigned char ErrorFlag;
  unsigned char W1VHigh;
  unsigned char W1High;
  unsigned char W1Low;
  unsigned char W2VHigh;
  unsigned char W2High;
  unsigned char W2Low;
  unsigned char W3VHigh;
  unsigned char W3High;
  unsigned char W3Low;
  unsigned char W4VHigh;
  unsigned char W4High;
  unsigned char W4Low;
  unsigned char W5VHigh;
  unsigned char W5High;
  unsigned char W5Low;
  unsigned char W6VHigh;
  unsigned char W6High;
  unsigned char W6Low;
  unsigned char W7VHigh;
  unsigned char W7High;
  unsigned char W7Low;
  unsigned char W8VHigh;
  unsigned char W8High;
  unsigned char W8Low;
  unsigned char W9VHigh;
  unsigned char W9High;
  unsigned char W9Low;
  unsigned char W10VHigh;
  unsigned char W10High;
  unsigned char W10Low;
  unsigned char W11VHigh;
  unsigned char W11High;
  unsigned char W11Low;
  unsigned char W12VHigh;
  unsigned char W12High;
  unsigned char W12Low;
  unsigned char VoltsInVHigh;
  unsigned char VoltsInHigh;
  unsigned char VoltsInLow;
  unsigned char Optostate1;
  unsigned char Optostate2;
  unsigned char OptoFreq1;
  unsigned char OptoFreq2;
  unsigned char OptoPW1;
  unsigned char OptoPW2;
  unsigned char DarkMode;
  unsigned char Temperature1;
  unsigned char Temperature2;
  unsigned char Temperature3;
  unsigned char Temperature4;
  unsigned char Humidity1;
  unsigned char Humidity2;
  unsigned char Humidity3;
  unsigned char Humidity4;
  unsigned char LightHigh;
  unsigned char LightLow;
  unsigned char Index1;
  unsigned char Index2;
  unsigned char Index3;
  unsigned char Index4;
  unsigned char Checksum1;
  unsigned char Checksum2;  
  unsigned char Checksum3;
  unsigned char Checksum4;    
};

#define STATUSPACKETSIZE 66

#endif	/* PACKETMANAGER_H */

