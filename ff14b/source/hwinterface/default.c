/*
 * default.c
 *
 *  Created on: 21 de nov de 2021
 *      Author: helder-sales
 */

#include "interface.h"

static int default_is_initialized(void);
static void default_select(void);
static void default_deselect(void);
static void default_spi_transmit_buffer(const BYTE *buff, UINT byteCount);
static void default_spi_transmit_byte(BYTE data);
static void default_spi_receive_buffer(BYTE *buff, UINT byteCount);
static BYTE default_spi_receive_byte(void);
static void default_spi_init(void);
static void default_spi_change_to_high_speed(void);
static void default_delay_100us_res(UINT delay);
static DWORD default_get_calendar_time(void);

HW_Interface_t HwInterface = {
    .is_initialized = default_is_initialized,
    .select = default_select,
    .deselect = default_deselect,
    .spi_transmit_buffer = default_spi_transmit_buffer,
    .spi_transmit_byte = default_spi_transmit_byte,
    .spi_receive_buffer = default_spi_receive_buffer,
    .spi_receive_byte = default_spi_receive_byte,
    .spi_init = default_spi_init,
    .spi_change_to_high_speed = default_spi_change_to_high_speed,
    .delay_100us_res = default_delay_100us_res,
    .get_calendar_time = default_get_calendar_time,
};

static int default_is_initialized(void) {
    return DEFAULT_INIT;
}

static void default_select(void) {
    while (1) {}
}

static void default_deselect(void) {
    while (1) {}
}

static void default_spi_transmit_buffer(const BYTE *buff, UINT byteCount) {
    while (1) {}
}

static void default_spi_transmit_byte(BYTE data) {
    while (1) {}
}

static void default_spi_receive_buffer(BYTE *buff, UINT byteCount) {
    while (1) {}
}

static BYTE default_spi_receive_byte(void) {
    while (1) {}
}

static void default_delay_100us_res(UINT delay) {
    while (1) {}
}

#if FF_FS_NORTC == 0 && FF_FS_READONLY == 0
/*
 * See http://elm-chan.org/fsw/ff/doc/fattime.html for return value
 */
static DWORD default_get_calendar_time(void) {
    while (1) {}
}

#else
/*
 * No RTC mode doesn't need the implementation, just return 0
 */
static DWORD default_get_calendar_time(void) {
    while (1) {}
}
#endif

static void default_spi_init(void) {
    while (1) {}
}

static void default_spi_change_to_high_speed(void) {
    while (1) {}
}
