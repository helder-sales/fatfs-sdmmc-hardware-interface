/*
 * msp430fr5994.c
 *
 *  Created on: 21 de mar de 2021
 *      Author: helder-sales
 */

#include "interface.h"
#include "driverlib.h"
#include "timer_interrupts.h"

#define SD_LAUNCHPAD
#define ENABLE_MISO_PULLUP

#if defined(SD_LAUNCHPAD)
#define EUSCI_SPI_BASE_ADDR EUSCI_B0_BASE

#else
#define EUSCI_SPI_BASE_ADDR EUSCI_B1_BASE
#endif

int msp430fr5994_is_initialized(void) {
    return IS_INIT;
}

#if defined(SD_LAUNCHPAD)
void msp430fr5994_select(void) {
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
}

void msp430fr5994_deselect(void) {
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);
}

#else
void msp430fr5994_select(void) {
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN1);
}

void msp430fr5994_deselect(void) {
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN1);
}
#endif

void msp430fr5994_spi_transmit_buffer(const BYTE *buff, UINT byteCount) {
    for (UINT i = 0; i < byteCount; i++) {
        EUSCI_B_SPI_transmitData(EUSCI_SPI_BASE_ADDR, *buff++);
        while (EUSCI_B_SPI_isBusy(EUSCI_SPI_BASE_ADDR)) {}
    }
}

void msp430fr5994_spi_transmit_byte(BYTE data) {
    EUSCI_B_SPI_transmitData(EUSCI_SPI_BASE_ADDR, data);
    while (EUSCI_B_SPI_isBusy(EUSCI_SPI_BASE_ADDR)) {}
}

void msp430fr5994_spi_receive_buffer(BYTE *buff, UINT byteCount) {
    for (UINT i = 0; i < byteCount; i++) {
        EUSCI_B_SPI_transmitData(EUSCI_SPI_BASE_ADDR, 0xFF);
        while (EUSCI_B_SPI_isBusy(EUSCI_SPI_BASE_ADDR)) {}
        *buff++ = EUSCI_B_SPI_receiveData(EUSCI_SPI_BASE_ADDR);
    }
}

BYTE msp430fr5994_spi_receive_byte(void) {
    EUSCI_B_SPI_transmitData(EUSCI_SPI_BASE_ADDR, 0xFF);
    while (EUSCI_B_SPI_isBusy(EUSCI_SPI_BASE_ADDR)) {}
    return EUSCI_B_SPI_receiveData(EUSCI_SPI_BASE_ADDR);
}

/*
 * In this case it is being used the MSP430 Timer Interrupt
 */
void msp430fr5994_delay_100us_res(UINT delay) {
    g_ticks = 0;

    while (g_ticks <= delay) {
        g_wake = true;
        __bis_SR_register(LPM3_bits);
    }
}

#if FF_FS_NORTC == 0 && FF_FS_READONLY == 0
/*
 * See http://elm-chan.org/fsw/ff/doc/fattime.html for return value
 */
DWORD msp430fr5994_get_calendar_time(void) {
    Calendar CurrentTime = RTC_C_getCalendarTime(RTC_C_BASE);

    return (DWORD) (CurrentTime.Year - 1980) << 25 |
           (DWORD) (CurrentTime.Month) << 21 |
           (DWORD) (CurrentTime.DayOfMonth) << 16 |
           (DWORD) (CurrentTime.Hours) << 11 |
           (DWORD) (CurrentTime.Minutes) << 5 |
           (DWORD) (CurrentTime.Seconds) >> 1;
}

#else
/*
 * No RTC mode doesn't need the implementation, just return 0
 */
DWORD msp430fr5994_get_calendar_time(void) {
    return 0;
}
#endif

#if defined(SD_LAUNCHPAD)
void msp430fr5994_spi_init(void) {
    // MOSI
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN6,
                                                GPIO_SECONDARY_MODULE_FUNCTION);
    // SCK
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN2,
                                                GPIO_SECONDARY_MODULE_FUNCTION);
    // MISO
#if defined(ENABLE_MISO_PULLUP)
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN7);
#endif
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN7,
                                               GPIO_SECONDARY_MODULE_FUNCTION);
    // CS
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);

    EUSCI_B_SPI_initMasterParam SpiInitMasterParam = {
        .selectClockSource = EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
        .clockSourceFrequency = 8000000,
        .desiredSpiClock = 400000,
        .msbFirst = EUSCI_B_SPI_MSB_FIRST,
        .clockPhase = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
        .clockPolarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW,
        .spiMode = EUSCI_B_SPI_3PIN,
    };

    EUSCI_B_SPI_initMaster(EUSCI_SPI_BASE_ADDR, &SpiInitMasterParam);
    EUSCI_B_SPI_enable(EUSCI_SPI_BASE_ADDR);
}

#else
void msp430fr5994_spi_init(void) {
    // MOSI
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5, GPIO_PIN0,
                                                GPIO_PRIMARY_MODULE_FUNCTION);
    // SCK
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5, GPIO_PIN2,
                                                GPIO_PRIMARY_MODULE_FUNCTION);
    // MISO
#if defined(ENABLE_MISO_PULLUP)
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN1);
#endif
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN1,
                                               GPIO_PRIMARY_MODULE_FUNCTION);
    // CS
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN1);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN1);

    EUSCI_B_SPI_initMasterParam SpiInitMasterParam = {
        .selectClockSource = EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
        .clockSourceFrequency = 8000000,
        .desiredSpiClock = 400000,
        .msbFirst = EUSCI_B_SPI_MSB_FIRST,
        .clockPhase = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
        .clockPolarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW,
        .spiMode = EUSCI_B_SPI_3PIN,
    };

    EUSCI_B_SPI_initMaster(EUSCI_SPI_BASE_ADDR, &SpiInitMasterParam);
    EUSCI_B_SPI_enable(EUSCI_SPI_BASE_ADDR);
}
#endif

void msp430fr5994_spi_change_to_high_speed(void) {
    EUSCI_B_SPI_changeMasterClockParam SpiChangeMasterClockParam = {
        .clockSourceFrequency = 8000000, .desiredSpiClock = 4000000};

    EUSCI_B_SPI_changeMasterClock(EUSCI_SPI_BASE_ADDR,
                                  &SpiChangeMasterClockParam);
}
