/*
 * interface.h
 *
 *  Created on: 21 de mar de 2021
 *      Author: helder-sales
 */

#ifndef FF14B_SOURCE_HWINTERFACE_INTERFACE_H_
#define FF14B_SOURCE_HWINTERFACE_INTERFACE_H_

#include "../ff.h"

typedef struct {
    int (*is_initialized)(void);
    void (*select)(void);
    void (*deselect)(void);
    void (*spi_transmit_buffer)(const BYTE *buff, UINT byteCount);
    void (*spi_transmit_byte)(BYTE data);
    void (*spi_receive_buffer)(BYTE *buff, UINT byteCount);
    BYTE (*spi_receive_byte)(void);
    void (*spi_init)(void);
    void (*spi_change_to_high_speed)(void);
    void (*delay_100us_res)(UINT delay);
    DWORD (*get_calendar_time)(void);
} HW_Interface_t;

typedef enum
{
    DEFAULT_INIT = -1,
    INIT_NOK = -1,
    IS_INIT = 0,
    INIT_OK = 0
} InitStatus;

extern HW_Interface_t HwInterface;

#endif /* FF14B_SOURCE_HWINTERFACE_INTERFACE_H_ */
