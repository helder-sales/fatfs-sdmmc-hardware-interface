/*------------------------------------------------------------------------/
 /  Foolproof MMCv3/SDv1/SDv2 (in SPI mode) control module
 /-------------------------------------------------------------------------/
 /
 /  Copyright (C) 2019, ChaN, all right reserved.
 /
 / * This software is a free software and there is NO WARRANTY.
 / * No restriction on use. You can use, modify and redistribute it for
 /   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
 / * Redistributions of source code must retain the above copyright notice.
 /
 /-------------------------------------------------------------------------/

 Modified on 21/03/2021 by helder-sales

 Features and Limitations:

 * No Media Change Detection
 Application program needs to perform a f_mount() after media change.

 /-------------------------------------------------------------------------*/

#include "../hwinterface/interface.h" /* Hardware interface */
#include "../ff.h"                    /* Obtains integer types for FatFs */
#include "../diskio.h" /* Common include file for FatFs and disk I/O layer */
#include "sdmmc.h"

/* MMC/SD command (SPI mode) */
#define CMD0  (0x40 + 0)  /* GO_IDLE_STATE */
#define CMD1  (0x40 + 1)  /* SEND_OP_COND */
#define CMD8  (0x40 + 8)  /* SEND_IF_COND */
#define CMD9  (0x40 + 9)  /* SEND_CSD */
#define CMD10 (0x40 + 10) /* SEND_CID */
#define CMD12 (0x40 + 12) /* STOP_TRANSMISSION */
#define CMD16 (0x40 + 16) /* SET_BLOCKLEN */
#define CMD17 (0x40 + 17) /* READ_SINGLE_BLOCK */
#define CMD18 (0x40 + 18) /* READ_MULTIPLE_BLOCK */
#define CMD23 (0x40 + 23) /* SET_BLOCK_COUNT */
#define CMD24 (0x40 + 24) /* WRITE_BLOCK */
#define CMD25 (0x40 + 25) /* WRITE_MULTIPLE_BLOCK */
#define CMD41 (0x40 + 41) /* SEND_OP_COND (ACMD) */
#define CMD55 (0x40 + 55) /* APP_CMD */
#define CMD58 (0x40 + 58) /* READ_OCR */

/*--------------------------------------------------------------------------

 Module Private Functions and Variables

 ---------------------------------------------------------------------------*/

/* Disk status */
static DSTATUS Stat = STA_NOINIT;
/* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */
static BYTE cardType;

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/
/* 1:OK, 0:Timeout */
static int wait_ready(void) {
    UINT tmr;

    /* Wait for ready in timeout of 500ms */
    for (tmr = 5000; tmr; tmr--) {
        if (HwInterface.spi_receive_byte() == 0xFF)
            break;

        HwInterface.delay_100us_res(1);
    }

    return tmr ? 1 : 0;
}

/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/
/* 1:OK, 0:Timeout */
static int select_wait_ready(void) {
    HwInterface.select();

    HwInterface.delay_100us_res(1);

    /* Dummy clock (force DO enabled) */
    HwInterface.spi_transmit_byte(0xFF);

    /* Wait for card ready */
    if (wait_ready())
        return 1;

    HwInterface.deselect();

    /* Failed */
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Power On                                                              */
/*-----------------------------------------------------------------------*/
static void sd_power_on(void) {
    BYTE args[6];
    DWORD cnt = 0x1FFF;

    /* transmit bytes to wake up */
    HwInterface.deselect();

    for (UINT i = 0; i < 10; i++)
        HwInterface.spi_transmit_byte(0xFF);

    /* slave Select */
    HwInterface.select();

    /* make idle state */
    args[0] = CMD0; /* CMD0:GO_IDLE_STATE */
    args[1] = 0;
    args[2] = 0;
    args[3] = 0;
    args[4] = 0;
    args[5] = 0x95; /* CRC */

    HwInterface.spi_transmit_buffer(args, sizeof args);

    /* wait response */
    while ((HwInterface.spi_receive_byte() != 0x01) && cnt)
        cnt--;

    HwInterface.deselect();

    HwInterface.spi_transmit_byte(0XFF);
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the card                                   */
/*-----------------------------------------------------------------------*/
/* 1:OK, 0:Failed */
static int spi_receive_data_block(BYTE *buff, UINT btr) {
    BYTE d;
    UINT tmr;

    /* Wait for data packet in timeout of 100ms */
    for (tmr = 1000; tmr; tmr--) {
        d = HwInterface.spi_receive_byte();

        if (d != 0xFF)
            break;

        HwInterface.delay_100us_res(1);
    }

    /* If not valid data token, return with error */
    if (d != 0xFE)
        return 0;

    /* Receive the data block into buffer */
    HwInterface.spi_receive_buffer(buff, btr);
    /* Discard CRC */
    HwInterface.spi_transmit_buffer((BYTE[]){0xFF, 0xFF}, 2);
    /* Return with success */
    return 1;
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to the card                                        */
/*-----------------------------------------------------------------------*/
/* 1:OK, 0:Failed */
// buff - /* 512 byte data block to be transmitted */
// token - /* Data/Stop token */
static int spi_transmit_data_block(const BYTE *buff, BYTE token) {
    if (!wait_ready())
        return 0;

    HwInterface.spi_transmit_byte(token);

    /* Is it data token? */
    if (token != 0xFD) {
        /* Xmit the 512 byte data block to MMC */
        HwInterface.spi_transmit_buffer(buff, 512);
        /* Xmit dummy CRC (0xFF, 0xFF) */
        HwInterface.spi_transmit_buffer((BYTE[]){0xFF, 0xFF}, 2);
        /* Receive data response */
        BYTE d = HwInterface.spi_receive_byte();

        /* If not accepted, return with error */
        if ((d & 0x1F) != 0x05)
            return 0;
    }

    return 1;
}

/*-----------------------------------------------------------------------*/
/* Send a command packet to the card                                     */
/*-----------------------------------------------------------------------*/
/* Returns command response (bit7==1:Send failed)*/
static BYTE send_cmd(BYTE cmd, DWORD arg) {
    BYTE crc;
    BYTE res;

    /* wait SD ready */
    if (wait_ready() != 1)
        return 0xFF;

    /* transmit command */
    /* Command */
    HwInterface.spi_transmit_byte(cmd);
    /* Argument[31..24] */
    HwInterface.spi_transmit_byte((BYTE) (arg >> 24));
    /* Argument[23..16] */
    HwInterface.spi_transmit_byte((BYTE) (arg >> 16));
    /* Argument[15..8] */
    HwInterface.spi_transmit_byte((BYTE) (arg >> 8));
    /* Argument[7..0] */
    HwInterface.spi_transmit_byte((BYTE) arg);

    /* prepare CRC */
    /* CRC for CMD0(0) */
    if (cmd == CMD0)
        crc = 0x95;

    /* CRC for CMD8(0x1AA) */
    else if (cmd == CMD8)
        crc = 0x87;

    else
        crc = 1;

    /* transmit CRC */
    HwInterface.spi_transmit_byte(crc);

    /* Skip a stuff byte when STOP_TRANSMISSION */
    if (cmd == CMD12)
        HwInterface.spi_transmit_byte(0xFF);

    /* receive response */
    BYTE n = 10;

    do
        res = HwInterface.spi_receive_byte();
    while ((res & 0x80) && --n);

    return res;
}

/*--------------------------------------------------------------------------

 Public Functions

 ---------------------------------------------------------------------------*/

int initialize_fatfs_interface(HW_Interface_t interface) {
    HwInterface = interface;

    if (HwInterface.is_initialized() == IS_INIT)
        return INIT_OK;

    else
        return INIT_NOK;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status(BYTE drv /* Drive number (always 0) */) {
    if (drv)
        return STA_NOINIT;

    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
/* Physical drive nmuber (0) */
DSTATUS disk_initialize(BYTE drv) {
    BYTE ty;
    BYTE buf[4];
    UINT tmr;
    DSTATUS s;

    if (drv)
        return RES_NOTRDY;

    // 10ms
    HwInterface.delay_100us_res(100);
    HwInterface.spi_init();
    sd_power_on();
    HwInterface.select();
    ty = 0;

    /* Enter Idle state */
    if (send_cmd(CMD0, 0) == 1) {
        /* SDv2? */
        if (send_cmd(CMD8, 0x1AA) == 1) {
            HwInterface.spi_receive_buffer(buf, 4);

            /* Get trailing return value of R7 resp */
            if (buf[2] == 0x01 && buf[3] == 0xAA) {
                /* The card can work at vdd range of 2.7-3.6V */
                /* Wait for leaving idle state (ACMD41 with HCS bit) */
                for (tmr = 1000; tmr; tmr--) {
                    if (send_cmd(CMD55, 0) <= 1 &&
                        send_cmd(CMD41, 1UL << 30) == 0)
                        break;

                    // 1ms
                    HwInterface.delay_100us_res(10);
                }

                /* Check CCS bit in the OCR */
                if (tmr && send_cmd(CMD58, 0) == 0) {
                    HwInterface.spi_receive_buffer(buf, 4);
                    ty = (buf[0] & 0x40) ? CT_SDC2 | CT_BLOCK
                                         : CT_SDC2; /* SDv2+ */
                }
            }
        }

        /* SDv1 or MMCv3 */
        else {
            /* SDv1 */
            if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 1UL << 30) == 0)
                ty = CT_SDC2;

            /* MMCv3 */
            else
                ty = CT_MMC3;

            /* Wait for leaving idle state */
            for (tmr = 1000; tmr; tmr--) {
                if (ty == CT_SDC2) {
                    if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) == 0)
                        break;
                } else if (ty == CT_MMC3) {
                    if (send_cmd(CMD1, 0) == 0)
                        break;
                }

                // 1ms
                HwInterface.delay_100us_res(10);
            }

            if (!tmr ||
                send_cmd(CMD16, 512) != 0) /* Set R/W block length to 512 */
                ty = 0;
        }
    }

    cardType = ty;
    s = ty ? 0 : STA_NOINIT;
    Stat = s;
    HwInterface.deselect();
    HwInterface.spi_change_to_high_speed();
    return s;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
/* Physical drive nmuber (0) */
/* Pointer to the data buffer to store read data */
/* Start sector number (LBA) */
/* Sector count (1..128) */
DRESULT disk_read(BYTE drv, BYTE *buff, LBA_t sector, UINT count) {
    BYTE cmd;
    DWORD sect = (DWORD) sector;

    if (disk_status(drv) & STA_NOINIT)
        return RES_NOTRDY;

    /* Convert LBA to byte address if needed */
    if (!(cardType & CT_BLOCK))
        sect *= 512;

    /*  READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK */
    cmd = count > 1 ? CMD18 : CMD17;
    HwInterface.select();

    if (send_cmd(cmd, sect) == 0) {
        do {
            if (!spi_receive_data_block(buff, 512))
                break;

            buff += 512;
        } while (--count);

        /* STOP_TRANSMISSION */
        if (cmd == CMD18)
            send_cmd(CMD12, 0);
    }

    HwInterface.deselect();
    return count ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/* Physical drive nmuber (0) */
/* Pointer to the data to be written */
/* Start sector number (LBA) */
/* Sector count (1..128) */
DRESULT disk_write(BYTE drv, const BYTE *buff, LBA_t sector, UINT count) {
    DWORD sect = (DWORD) sector;

    if (disk_status(drv) & STA_NOINIT)
        return RES_NOTRDY;

    /* Convert LBA to byte address if needed */
    if (!(cardType & CT_BLOCK))
        sect *= 512;

    HwInterface.select();

    /* Single block write */
    if (count == 1) {
        /* WRITE_BLOCK */
        if ((send_cmd(CMD24, sect) == 0) && spi_transmit_data_block(buff, 0xFE))
            count = 0;
    }

    /* Multiple block write */
    else {
        if (cardType & CT_SDC) {
            send_cmd(CMD55, 0);
            send_cmd(CMD23, count); /* ACMD23 */
        }

        /* WRITE_MULTIPLE_BLOCK */
        if (send_cmd(CMD25, sect) == 0) {
            do {
                if (!spi_transmit_data_block(buff, 0xFC))
                    break;

                buff += 512;
            } while (--count);

            /* STOP_TRAN token */
            if (!spi_transmit_data_block(0, 0xFD))
                count = 1;
        }
    }

    HwInterface.deselect();
    return count ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
/* Physical drive nmuber (0) */
/* Control code */
/* Buffer to send/receive control data */
DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff) {
    DRESULT res;
    BYTE n;
    BYTE csd[16];
    DWORD cs;

    /* Check if card is in the socket */
    if (disk_status(drv) & STA_NOINIT)
        return RES_NOTRDY;

    res = RES_ERROR;

    switch (ctrl) {
    /* Make sure that no pending write process */
    case CTRL_SYNC:
        if (select_wait_ready())
            res = RES_OK;
        break;

        /* Get number of sectors on the disk (DWORD) */
    case GET_SECTOR_COUNT:
        HwInterface.select();

        if ((send_cmd(CMD9, 0) == 0) && spi_receive_data_block(csd, 16)) {
            /* SDC ver 2.00 */
            if ((csd[0] >> 6) == 1) {
                cs = csd[9] + ((WORD) csd[8] << 8) +
                     ((DWORD) (csd[7] & 63) << 16) + 1;
                *(LBA_t *) buff = cs << 10;
            }

            /* SDC ver 1.XX or MMC */
            else {
                n = (csd[5] & 15) + ((csd[10] & 128) >> 7) +
                    ((csd[9] & 3) << 1) + 2;
                cs = (csd[8] >> 6) + ((WORD) csd[7] << 2) +
                     ((WORD) (csd[6] & 3) << 10) + 1;
                *(LBA_t *) buff = cs << (n - 9);
            }

            res = RES_OK;
        }
        break;

        /* Get erase block size in unit of sector (DWORD) */
    case GET_BLOCK_SIZE:
        *(DWORD *) buff = 128;
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
        break;
    }

    HwInterface.deselect();
    return res;
}

#if FF_FS_NORTC == 0 && FF_FS_READONLY == 0
DWORD get_fattime(void) {
    return HwInterface.get_calendar_time();
}
#endif
