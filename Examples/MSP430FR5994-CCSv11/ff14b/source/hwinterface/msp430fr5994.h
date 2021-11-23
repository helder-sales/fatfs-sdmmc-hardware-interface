/*
 * msp430fr5994.h
 *
 *  Created on: 21 de nov de 2021
 *      Author: helder-sales
 */

#ifndef FF14B_SOURCE_HWINTERFACE_MSP430FR5994_H_
#define FF14B_SOURCE_HWINTERFACE_MSP430FR5994_H_

int msp430fr5994_is_initialized(void);
void msp430fr5994_select(void);
void msp430fr5994_deselect(void);
void msp430fr5994_spi_transmit_buffer(const BYTE *buff, UINT byteCount);
void msp430fr5994_spi_transmit_byte(BYTE data);
void msp430fr5994_spi_receive_buffer(BYTE *buff, UINT byteCount);
BYTE msp430fr5994_spi_receive_byte(void);
void msp430fr5994_spi_init(void);
void msp430fr5994_spi_change_to_high_speed(void);
void msp430fr5994_delay_100us_res(UINT delay);
DWORD msp430fr5994_get_calendar_time(void);

#endif /* FF14B_SOURCE_HWINTERFACE_MSP430FR5994_H_ */
