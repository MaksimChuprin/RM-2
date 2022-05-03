#include "USB_Task.h"
#include "RTOS.h"
#include "stm32f1xx.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_customhid.h"
#include "usbd_customhid_if.h"
#include "usbd_cdc_if_template.h"
#include "device_struct.h"

#if     defined(STM32F103xG)
  #define __DISABLE_USB_GLOBAL      { __HAL_PCD_DISABLE(&hpcd); HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn); }
  #define __ENABLE_USB_GLOBAL       { __HAL_PCD_ENABLE( &hpcd); HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);  }
#elif   defined(STM32F107xC)
  #define __DISABLE_USB_GLOBAL      __HAL_PCD_DISABLE(&hpcd)
  #define __ENABLE_USB_GLOBAL       __HAL_PCD_ENABLE( &hpcd)
#else
  #error  "CPU Not defined!"
#endif

#ifndef HW_RESET
  #define HW_RESET                  *((volatile unsigned long *)(0xE000ED0C)) = 0x05FA0004
#endif

/* macros privat */
static OS_EVENT                   usbEvent;
USBD_HandleTypeDef                USBD_Device;

// externs
extern PCD_HandleTypeDef          hpcd;
extern volatile system_flags_t    system_flags;

/* USB_Stop */
void USB_Stop(void)
{
  USBD_Stop(&USBD_Device);
}

/* USB_Deinit */
void USB_DeInit(void)
{
  USBD_DeInit(&USBD_Device);
  __DISABLE_USB_GLOBAL;
}

/* USB_Connect */
bool USB_IsConfigured(void)
{
  return (USBD_Device.dev_state == USBD_STATE_CONFIGURED);
}

/* ISR */
void USB_ISR_Handler(void) 
{  
  OS_EnterInterrupt();
    
  OS_EVENT_Set( &usbEvent );
  hpcd.Lock = HAL_LOCKED;
  __DISABLE_USB_GLOBAL;
  
  OS_LeaveInterrupt();
}

/* Task */
void USB_Task(void) 
{      
  OS_EVENT_Create ( &usbEvent );
  
  for(;;OS_Delay(100))
  {
    if( IS_USB_CONNECT )
    {  
      /* Init Device Library */
      USBD_Init( &USBD_Device, &HID_Desc, 0);
  
      /* Add Supported Class */
      USBD_RegisterClass( &USBD_Device, &USBD_CUSTOM_HID);
  
      /* Add Custom HID callbacks */
      USBD_CUSTOM_HID_RegisterInterface( &USBD_Device, &USBD_CustomHID_fops);
      USBD_CDC_RegisterInterface( &USBD_Device, &USBD_CDC_Template_fops);
  
      /* Start Device Process */
      USBD_Start(&USBD_Device);
      __ENABLE_USB_GLOBAL;
  
      // main USB loop    
      for(U8 i = 0; IS_USB_CONNECT; ) 
      {         
        OS_EVENT_WaitTimed( &usbEvent, 50);
        
        if( hpcd.Lock == HAL_LOCKED )
        {
          hpcd.Lock = HAL_UNLOCKED;
          HAL_PCD_IRQHandler( &hpcd);
          __ENABLE_USB_GLOBAL;
        }
        else 
        {
          if(system_flags.start_boot) i++;
          if( i >= 8 )
          {           
            USB_DeInit();
              OS_Delay(100);
                HW_RESET;
          }
        }
      } // for(U8 i = 0;IS_USB_CONNECT;)
      
      USB_DeInit();
    } // if( IS_USB_CONNECT )
  } // for(;;)
}
