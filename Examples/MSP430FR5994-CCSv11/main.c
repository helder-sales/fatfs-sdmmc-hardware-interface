#include "driverlib.h"
#include "ff14b/source/ff.h" /* Declarations of FatFs API */
#include "ff14b/source/hwinterface/interface.h"
#include "ff14b/source/hwinterface/msp430fr5994.h"
#include "ff14b/source/sdmmc/sdmmc.h"
#include "timer_interrupts.h"
#include "peripherals/rtc.h"
#include "peripherals/timer.h"

int main(void) {
    // Stop WDT
    WDT_A_hold(WDT_A_BASE);

    // Disable the GPIO power-on default high-impedance mode
    // to activate port settings
    PMM_unlockLPM5();

    GPIO_setAsOutputPin(GPIO_PORT_PJ, GPIO_PIN_ALL16);
    GPIO_setOutputLowOnPin(GPIO_PORT_PJ, GPIO_PIN_ALL16);

    // Set PJ.4 and PJ.5 as Primary Module Function Input, LFXT.
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_PJ, GPIO_PIN4 | GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);

    // Configure FRAM wait state for 16 MHz
    FRAMCtl_A_configureWaitStateControl(FRAMCTL_A_ACCESS_TIME_CYCLES_1);

    // Set DCO to 16 MHz
    CS_setDCOFreq(CS_DCORSEL_1, CS_DCOFSEL_4);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    // Configure SMCLK and ACLK
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
    CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);

    // Configure LEDs to show program status
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0 | GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);

    TIM_Config();

#if FF_FS_NORTC == 0 && FF_FS_READONLY == 0
    RTC_Config();
#endif

    HW_Interface_t Msp430Fr5994Interface = {
        .is_initialized = msp430fr5994_is_initialized,
        .select = msp430fr5994_select,
        .deselect = msp430fr5994_deselect,
        .spi_transmit_buffer = msp430fr5994_spi_transmit_buffer,
        .spi_transmit_byte = msp430fr5994_spi_transmit_byte,
        .spi_receive_buffer = msp430fr5994_spi_receive_buffer,
        .spi_receive_byte = msp430fr5994_spi_receive_byte,
        .spi_init = msp430fr5994_spi_init,
        .spi_change_to_high_speed = msp430fr5994_spi_change_to_high_speed,
        .delay_100us_res = msp430fr5994_delay_100us_res,
        .get_calendar_time = msp430fr5994_get_calendar_time,
    };

    if (initialize_fatfs_interface(Msp430Fr5994Interface) != INIT_OK)
        while (1) {}

    __bis_SR_register(GIE);

    // FatFs work area needed for each volume
    FATFS fatfs;
    // File object needed for each open file
    FIL fil;
    // Return code
    FRESULT rc;

    rc = f_mount(&fatfs, "", 0);

    if (rc == FR_OK) {
        rc = f_open(&fil, "new_file.txt", FA_WRITE | FA_OPEN_APPEND);

        if (rc == FR_OK) {
            f_puts("Some random text\r\n", &fil);

            rc = f_close(&fil);

            if (rc == FR_OK)
                rc = f_unmount("");
        }
    }

    if (rc == FR_OK) {
        while (1) {
            GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN1);
            DELAY_MS(1000);
        }
    } else {
        while (1) {
            GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            DELAY_MS(1000);
        }
    }
}
