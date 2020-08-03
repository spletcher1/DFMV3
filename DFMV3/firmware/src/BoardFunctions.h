/* 
 * File:   BoardFunctions.h
 * Author: scott
 *
 * Created on June 6, 2020, 10:34 AM
 */

#ifndef BOARDFUNCTIONS_H
#define	BOARDFUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

void InitializeBoard();
void SetDarkMode(unsigned char mode);
unsigned char GetUsingNewPortOnly();


#ifdef	__cplusplus
}
#endif

#endif	/* BOARDFUNCTIONS_H */

