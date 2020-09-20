#ifndef OPTOFUNCTIONS_H
#define	OPTOFUNCTIONS_H


void ConfigureOpto();
void SetOptoState(unsigned int os);
void SetOptoParameters(unsigned int hz, unsigned int pw);
void SetPulseWidth_ms(unsigned int pw);
void SetHertz(unsigned int hz);
void TogglePortUse(void);

// Definitions for legacy port
#define COL1ON() LATGSET=0x100
#define COL1OFF() LATGCLR=0x100
#define COL2ON() LATDSET=0x10
#define COL2OFF() LATDCLR=0x10

#define ROW1ON() LATDSET=0x200
#define ROW1OFF() LATDCLR=0x200
#define ROW2ON() LATDSET=0x80
#define ROW2OFF() LATDCLR=0x80
#define ROW3ON() LATDSET=0x40
#define ROW3OFF() LATDCLR=0x40
#define ROW4ON() LATDSET=0x100
#define ROW4OFF() LATDCLR=0x100
#define ROW5ON() LATGSET=0x40
#define ROW5OFF() LATGCLR=0x40
#define ROW6ON() LATGSET=0x80
#define ROW6OFF() LATGCLR=0x80

#endif	/* OPTOFUNCTIONS_H */

