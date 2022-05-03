/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid_if.h"
#include "USB_Task.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static int8_t CustomHID_Init     (void);
static int8_t CustomHID_DeInit   (void);
static int8_t CustomHID_InEvent  (void);
static int8_t CustomHID_OutEvent (uint8_t *buf);

/* Private variables ---------------------------------------------------------*/

extern USBD_HandleTypeDef USBD_Device;

__ALIGN_BEGIN static uint8_t CustomHID_ReportDesc[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
  0x06, 0xFF, 0x00,      /* USAGE_PAGE (Vendor Page: 0xFF00) */                       
  0x09, 0x01,            /* USAGE (Demo Kit)               */    
  0xa1, 0x01,            /* COLLECTION (Application)       */            
  /* 7 */
       
//  0x85, 0x01,            /*     REPORT_ID (1)		   */
  0x09, 0x01,            /*     USAGE (LED 1)	           */
  0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */          
  0x26, 0xff, 0x00,      /*     LOGICAL_MAXIMUM (255)      */           
  0x75, 0x08,            /*     REPORT_SIZE (8)            */    
  0x95, USB_REPORT_LEN,  /*     REPORT_COUNT ()           */   
  0xB1, 0x82,            /*     FEATURE (Data,Var,Abs,Vol) */     
  
  0xc0 	                 /*     END_COLLECTION	             */
}; 

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops = 
{
  CustomHID_ReportDesc,
  CustomHID_Init,
  CustomHID_DeInit,
  CustomHID_OutEvent,
  CustomHID_InEvent,  
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CustomHID_Init
  *         Initializes the CUSTOM HID media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CustomHID_Init(void)
{  
  /*
  Add your deinitialization code here 
  */  
  return (0);
}

/**
  * @brief  CustomHID_DeInit
  *         DeInitializes the CUSTOM HID media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CustomHID_DeInit(void)
{
  /*
  Add your deinitialization code here 
  */  
  return (0);
}

/**
  * @brief  CustomHID_OutEvent
  *         Manage the CUSTOM HID class Out Event    
  * @param buf: inp command
  */
static int8_t CustomHID_OutEvent  (uint8_t *buf)
{ 
  usb_exchange(buf);
  
  return (0);
}

/**
  * @brief  CustomHID_InEvent
  *         Manage the CUSTOM HID class In Event    
  */
static int8_t CustomHID_InEvent  (void)
{ 
  /*
  Add your deinitialization code here 
  */
  return (0);
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
