#ifndef DEFINES_H
  #define DEFINES_H

#include "RTOS.h"
#include "iostm32f107xx.h"
#include "arm_comm.h"
#include <stdint.h>
#include "stdbool.h"
#include <intrinsics.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include "string.h"
#include <math.h>
#include "eksis.h"
#include "device_struct.h"

#define SEC_IN_ADAY             (3600*24L)

// ConfigFlags bits
#define CURR_BUFF_SWEEP         (1L<<0)      
#define SAVE_BUFF_SWEEP         (1L<<1)
#define TEMP_PSE_NOISE          (1L<<2)
#define HUM_PSE_NOISE           (1L<<3)
#define IGNORE_TIMESTAMP        (1L<<4)
#define GSM_WIFI                (1L<<5)

#define DISPLAY_MASK            0x0000ff00
#define __SDD1309_WEO012864G    (0<<8)
#define __SDD1309_WEG012864M    (1<<8)
#define __SDD1305_CHINANONAME1  (2<<8)

// ErrorArray's bits

// mem addresses
// #define ConfigSizeF             *((pU32)0x0803f9f8)
#define ConfigCheckSumAdr       0x0803fa00
#define ConfigFlashBeginAddr    0x0803f800
#define ConfigCheckSumF         *((volatile unsigned long *)(ConfigCheckSumAdr))

#define ConfigBeginAddr         0x2000be00
#define ConfigEndAddr           0x2000bfff
#define RamBeginAddr            0x2000c000
#define RamEndAddr              0x2000ffff
#define CONFIGSIZE              (RamBeginAddr - ConfigBeginAddr)
#define RAMSIZE                 (RamEndAddr - RamBeginAddr + 1)
#define CONFIG(A)               (ConfigBeginAddr + (A))
#define RAM(A)                  (RamBeginAddr + (A))
  
#define RAM_RAM_ADR(A)          (U16)((pU8)(A) - (pU8)RamBeginAddr)
#define CONFIG_ROM_ADR(A)       (U16)((pU8)(A) - (pU8)ConfigBeginAddr)

#define OS_SUSPEND_IF_NOT(A)    if(!OS_GetSuspendCnt((A))) OS_Suspend((A))
#define OS_RESUME_IF_NOT(A)     if(OS_GetSuspendCnt((A)))  OS_Resume((A))
#define CLEAR_IWDG              IWDG_KR = 0xAAAA
#define HW_RESET                *((volatile unsigned long *)(0xE000ED0C)) = 0x05FA0004
#define OS_STOPTIMER_IF_NOT(A)  if(OS_GetTimerStatus((A))) OS_StopTimer((A))
#define OS_STARTTIMER_IF_NOT(A) if(!OS_GetTimerStatus((A))) OS_StartTimer((A))

#define LIGHT_RX                GPIOE_BSRR = (1<<13)
#define LIGHT_TX                GPIOE_BSRR = (1<<14)
#define CHECK_TX                (GPIOE_ODR & (1<<14))
#define SWOFF_RX                GPIOE_BRR  = (1<<13)
#define SWOFF_TX                GPIOE_BRR  = (1<<14)

#include "UARTs_task.h"
#include "USB_Task.h"
#include "OLED.h"
#include "F868_task.h"
#include "WIFI_task.h"
#include "OS_IP_task.h"
#include "OS_MQTT_task.h"
#include "utilites.h"
#include "modbus.h"
#include "gsm.h"
#include "global_var.h"

#endif