/*
 * timer_interrupts.h
 *
 *  Created on: 27 de fev de 2021
 *      Author: helde
 */

#ifndef INTERRUPTS_TIMER_INTERRUPTS_H_
#define INTERRUPTS_TIMER_INTERRUPTS_H_

#include <msp430.h>
#include "driverlib.h"

#define TIME_100USRES(x) ((x))
#define TIME_MS(x) ((x)*10)
#define DELAY_100USRES(x)                                                      \
    do {                                                                       \
        g_ticks = 0;                                                           \
        while (g_ticks <= TIME_100USRES(x)) {                                  \
            g_wake = true;                                                     \
            __bis_SR_register(LPM3_bits);                                      \
        }                                                                      \
    } while (0)
#define DELAY_MS(x)                                                            \
    do {                                                                       \
        g_ticks = 0;                                                           \
        while (g_ticks <= TIME_MS(x)) {                                        \
            g_wake = true;                                                     \
            __bis_SR_register(LPM3_bits);                                      \
        }                                                                      \
    } while (0)

extern uint32_t g_ticks;
extern bool g_wake;

#endif /* INTERRUPTS_TIMER_INTERRUPTS_H_ */
