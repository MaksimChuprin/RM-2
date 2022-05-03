/*******************************************************************
** File        : XE1205driver.h                                   **
********************************************************************
**                                                                **
** Version     : V 1.0                                            **
**                                                                **
** Written by   : Miguel Luis & Grégoire Guye                     **
**                                                                **
** Date        : 19-01-2004                                       **
**                                                                **
** Project     : API-1205                                         **
**                                                                **
********************************************************************
** Changes     : V 2.1 / MiL - 24-04-2004                         **
**                                                                **
**             : V 2.2 / MiL - 30-07-2004                         **
**               - Removed workaround for RX/TX switch FIFO clear **
**                 (chip correction)                              **
**                                                                **
** Changes     : V 2.3 / CRo - 06-06-2006                         **
**               - I/O Ports Definitions section updated          **
**                                                                **
** Changes     : V 2.4 / CRo - 09-01-2007                         **
**               - No change                                      **
**                                                                **
**                                                                **
********************************************************************
** Description : XE1205 transceiver drivers Implementation for the**
**               XE8000 family products (1205 buffered mode)      **
*******************************************************************/

#include "arm_comm.h"
#include <stdint.h>

#ifndef __SX1276DRIVER__
#define __SX1276DRIVER__

U8                                spi_write   (U8 byte);
void                              spi_nss     (U8 NSS);
void                              wait_ms     (U32 time);
void                              Radio_Reset (void);

#define NSS_HI                    GPIOC_BSRR = (1<<6)
#define NSS_LOW                   GPIOC_BRR  = (1<<6)
#define RES_HI                    { GPIOC_CRL &= 0x0fffffff; GPIOC_CRL |= 0x40000000; }
#define RES_LOW                   { GPIOC_CRL &= 0x0fffffff; GPIOC_CRL |= 0x20000000; }
#endif /* __SX1276DRIVER__ */
