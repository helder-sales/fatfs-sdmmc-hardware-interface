/*
 * rtc.h
 *
 *  Created on: 21 de mar de 2021
 *      Author: helde
 */

#ifndef PERIPHERALS_RTC_H_
#define PERIPHERALS_RTC_H_

#include "driverlib.h"

#if FF_FS_NORTC == 0 && FF_FS_READONLY == 0
void RTC_Config(void);
#endif

#endif /* PERIPHERALS_RTC_H_ */
