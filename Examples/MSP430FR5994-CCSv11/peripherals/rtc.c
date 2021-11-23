/*
 * rtc.c
 *
 *  Created on: 21 de mar de 2021
 *      Author: helde
 */

#include "rtc.h"

#if FF_FS_NORTC == 0 && FF_FS_READONLY == 0
void RTC_Config(void) {
    Calendar CalendarInit = {
        .Seconds = 1,
        .Minutes = 15,
        .Hours = 0,
        .DayOfWeek = 1,
        .DayOfMonth = 28,
        .Month = 2,
        .Year = 2021,
    };

    RTC_C_initCalendar(RTC_C_BASE, &CalendarInit, RTC_C_FORMAT_BINARY);
    RTC_C_startClock(RTC_C_BASE);
}
#endif
