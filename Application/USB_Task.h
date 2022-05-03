#ifndef __USB_HID_H
#define __USB_HID_H

#include "arm_comm.h"
#include <stdbool.h>

#define WR_USB                  0x00 
#define WF_USB                  0x01
#define WI_USB                  0x02
#define WP_USB                  0x03
#define WCOM1_USB               0x03
#define WCOM2_USB               0x04

#define RR_USB                  0x80
#define RF_USB                  0x81
#define RI_USB                  0x82
#define RP_USB                  0x83
#define RCOM1_USB               0x83
#define RCOM2_USB               0x84

#define IR_USB                  0x8F
#define BS_USB                  0x55
#define DW_USB                  0x56
#define DR_USB                  0x57
#define CT_USB                  0x77
#define RS_USB                  0x78
#define SS_USB                  0x79
#define SC_USB                  0x7A
#define CR_USB                  0x8E

#define CONNECT_USB_TIMEOUT     5000
#define CONFIG_WRITE_TIMEOUT    500

#define ASK_USBEXCH             0
#define BSY_USBEXCH             254
#define NAK_USBEXCH             255

#define USB_MAX_DATA_LEN        128
#define USB_REPORT_LEN          USB_MAX_DATA_LEN + 8

#define GPIOA_IDR               *((volatile unsigned long *)(0x40010808))
#define IS_USB_CONNECT          ( GPIOA_IDR & (1 << 9) )

void                            USB_ISR_Handler(void);
void                            USB_Task(void);
void                            USB_Stop(void);
void                            USB_DeInit(void);
bool                            USB_IsConfigured(void);
void                            usb_exchange(pU8 buffer);
#endif