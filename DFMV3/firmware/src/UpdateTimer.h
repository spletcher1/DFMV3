/* 
 * File:   UpdateTimer.h
 * Author: scott
 *
 * Created on June 6, 2020, 10:40 AM
 */

#ifndef UPDATETIMER_H
#define	UPDATETIMER_H

#ifdef	__cplusplus
extern "C" {
#endif

void ConfigureUpdateTimer(void);
void TIMER1_EventHandler(uint32_t status, uintptr_t context);


#ifdef	__cplusplus
}
#endif

#endif	/* UPDATETIMER_H */

