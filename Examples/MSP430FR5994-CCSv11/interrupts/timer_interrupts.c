/*
 * timer_interrupts.c
 *
 *  Created on: 27 de fev de 2021
 *      Author: helde
 */

#include "timer_interrupts.h"

uint32_t g_ticks;
bool g_wake;

#pragma vector = TIMER2_A0_VECTOR
__interrupt void Timer2_A0_ISR(void) {
    g_ticks++;

    if (g_wake) {
        g_wake = false;
        __bic_SR_register_on_exit(LPM3_bits);
    }
}
