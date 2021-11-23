/*
 * timer.c
 *
 *  Created on: 21 de mar de 2021
 *      Author: helde
 */

#include "timer.h"

void TIM_Config(void) {
    Timer_A_initUpModeParam InitUpParam = {
        .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
        .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_8,
        .timerPeriod = 100,
        .timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE,
        .captureCompareInterruptEnable_CCR0_CCIE =
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
        .timerClear = TIMER_A_DO_CLEAR,
        .startTimer = true,
    };

    Timer_A_initUpMode(TIMER_A2_BASE, &InitUpParam);
}
