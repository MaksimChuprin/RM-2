/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_cdc.h"
#include "RTOS.h"


/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_CUSTOM_HID_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_CUSTOM_HID_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_CUSTOM_HID_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  *USBD_CUSTOM_HID_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_CUSTOM_HID_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_CUSTOM_HID_EP0_RxReady (USBD_HandleTypeDef  *pdev);

static uint8_t  USBD_CUSTOM_HID_EP0_TxSent (USBD_HandleTypeDef  *pdev);
/**
  * @}
  */ 

/** @defgroup USBD_CUSTOM_HID_Private_Variables
  * @{
  */ 

USBD_ClassTypeDef  USBD_CUSTOM_HID = 
{
  USBD_CUSTOM_HID_Init,
  USBD_CUSTOM_HID_DeInit,
  USBD_CUSTOM_HID_Setup,
  USBD_CUSTOM_HID_EP0_TxSent,   /* EP0_TxSent */  
  USBD_CUSTOM_HID_EP0_RxReady,  /* EP0_RxReady */ /* STATUS STAGE IN */
  USBD_CDC_DataIn,
  USBD_CDC_DataOut,
  NULL, /*SOF */
  NULL,
  NULL,      
  USBD_CUSTOM_HID_GetCfgDesc,
  USBD_CUSTOM_HID_GetCfgDesc, 
  USBD_CUSTOM_HID_GetCfgDesc,
  USBD_CUSTOM_HID_GetDeviceQualifierDesc,
};

#define CDC0_COMM_INTERFACE                0              // Comm interface number of CDC0
#define CDC0_DATA_INTERFACE                1              // Data interface number of CDC0
#define CDC0_INTEP_ADDR                    0x82              // Interrupt Endpoint Address of CDC0
#define CDC0_OUTEP_ADDR                    0x01              // Output Endpoint Address of CDC0
#define CDC0_INEP_ADDR                    0x81              // Input Endpoint Address of CDC0

//Descriptor Type Values
#define DESC_TYPE_DEVICE                1       //Device Descriptor (Type 1)
#define DESC_TYPE_CONFIG                2       //Configuration Descriptor (Type 2)
#define DESC_TYPE_STRING                3       //String Descriptor (Type 3)
#define DESC_TYPE_INTERFACE             4       //Interface Descriptor (Type 4)
#define DESC_TYPE_ENDPOINT              5       //Endpoint Descriptor (Type 5)
#define DESC_TYPE_DEVICE_QUALIFIER      6       //Endpoint Descriptor (Type 6)
#define DESC_TYPE_IAD                   0x0B
#define DESC_TYPE_HUB                   0x29    //Hub Descriptor (Type 6)
#define DESC_TYPE_HID                   0x21    //HID Descriptor
#define DESC_TYPE_REPORT                0x22    //Report Descriptor
#define DESC_TYPE_PHYSICAL              0x23    //Physical Descriptor

//Bit definitions for EndpointDescriptor.EndpointFlags
#define EP_DESC_ATTR_TYPE_MASK  0x03        //Mask value for bits 1-0
#define EP_DESC_ATTR_TYPE_CONT  0x00        //Bit 1-0: 00 = Endpoint does control transfers
#define EP_DESC_ATTR_TYPE_ISOC  0x01        //Bit 1-0: 01 = Endpoint does isochronous transfers
#define EP_DESC_ATTR_TYPE_BULK  0x02        //Bit 1-0: 10 = Endpoint does bulk transfers
#define EP_DESC_ATTR_TYPE_INT   0x03        //Bit 1-0: 11 = Endpoint does interrupt transfers

//ENDPOINT_DESCRIPTOR structure
#define SIZEOF_ENDPOINT_DESCRIPTOR 0x07

#define HID0_REPORT_INTERFACE                2              // Report interface number of HID0

#define INTF_STRING_INDEX         5

/* USB CUSTOM_HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_CfgDesc[USB_CUSTOM_HID_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_CUSTOM_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x03,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xC0,         /*bmAttributes: self powered */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
          //Interface Association Descriptor
        0x08,                              // bLength
        DESC_TYPE_IAD,                     // bDescriptorType = 11
        CDC0_COMM_INTERFACE,               // bFirstInterface
        0x02,                              // bInterfaceCount
        0x02,                              // bFunctionClass (Communication Class)
        0x02,                              // bFunctionSubClass (Abstract Control Model)
        0x01,                              // bFunctionProcotol (V.25ter, Common AT commands)
        INTF_STRING_INDEX + 0,             // iInterface

        //INTERFACE DESCRIPTOR (9 bytes)
        0x09,                              // bLength: Interface Descriptor size
        DESC_TYPE_INTERFACE,               // bDescriptorType: Interface
        CDC0_COMM_INTERFACE,               // bInterfaceNumber
        0x00,                              // bAlternateSetting: Alternate setting
        0x01,                              // bNumEndpoints: Three endpoints used
        0x02,                              // bInterfaceClass: Communication Interface Class
        0x02,                              // bInterfaceSubClass: Abstract Control Model
        0x01,                              // bInterfaceProtocol: Common AT commands
        INTF_STRING_INDEX + 0,             // iInterface:

        //Header Functional Descriptor
        0x05,                                // bLength: Endpoint Descriptor size
        0x24,                                // bDescriptorType: CS_INTERFACE
        0x00,                                // bDescriptorSubtype: Header Func Desc
        0x10,                                // bcdCDC: spec release number
        0x01,

        //Call Managment Functional Descriptor
        0x05,                                // bFunctionLength
        0x24,                                // bDescriptorType: CS_INTERFACE
        0x01,                                // bDescriptorSubtype: Call Management Func Desc
        0x00,                                // bmCapabilities: D0+D1
        CDC0_DATA_INTERFACE,                // bDataInterface: 0

        //ACM Functional Descriptor
        0x04,                                // bFunctionLength
        0x24,                                // bDescriptorType: CS_INTERFACE
        0x02,                                // bDescriptorSubtype: Abstract Control Management desc
        0x02,                                // bmCapabilities

        // Union Functional Descriptor
        0x05,                               // Size, in bytes
        0x24,                               // bDescriptorType: CS_INTERFACE
        0x06,                                // bDescriptorSubtype: Union Functional Desc
        CDC0_COMM_INTERFACE,                // bMasterInterface -- the controlling intf for the union
        CDC0_DATA_INTERFACE,                // bSlaveInterface -- the controlled intf for the union

        //EndPoint Descriptor for Interrupt endpoint
        SIZEOF_ENDPOINT_DESCRIPTOR,         // bLength: Endpoint Descriptor size
        DESC_TYPE_ENDPOINT,                 // bDescriptorType: Endpoint
        CDC0_INTEP_ADDR,                    // bEndpointAddress: (IN2)
        EP_DESC_ATTR_TYPE_INT,                // bmAttributes: Interrupt
        0x40, 0x00,                         // wMaxPacketSize, 64 bytes
        0xFF,                                // bInterval

        //DATA INTERFACE DESCRIPTOR (9 bytes)
        0x09,                                // bLength: Interface Descriptor size
        DESC_TYPE_INTERFACE,                // bDescriptorType: Interface
        CDC0_DATA_INTERFACE,                // bInterfaceNumber
        0x00,                               // bAlternateSetting: Alternate setting
        0x02,                               // bNumEndpoints: Three endpoints used
        0x0A,                               // bInterfaceClass: Data Interface Class
        0x00,                               // bInterfaceSubClass:
        0x00,                               // bInterfaceProtocol: No class specific protocol required
        0x00,                                // iInterface:

        //EndPoint Descriptor for Output endpoint
        SIZEOF_ENDPOINT_DESCRIPTOR,         // bLength: Endpoint Descriptor size
        DESC_TYPE_ENDPOINT,                    // bDescriptorType: Endpoint
        CDC0_OUTEP_ADDR,                    // bEndpointAddress: (OUT3)
        EP_DESC_ATTR_TYPE_BULK,                // bmAttributes: Bulk
        0x40, 0x00,                         // wMaxPacketSize, 64 bytes
        0xFF,                                 // bInterval: ignored for Bulk transfer

        //EndPoint Descriptor for Input endpoint
        SIZEOF_ENDPOINT_DESCRIPTOR,         // bLength: Endpoint Descriptor size
        DESC_TYPE_ENDPOINT,                    // bDescriptorType: Endpoint
        CDC0_INEP_ADDR,                        // bEndpointAddress: (IN3)
        EP_DESC_ATTR_TYPE_BULK,                // bmAttributes: Bulk
        0x40, 0x00,                         // wMaxPacketSize, 64 bytes
        0xFF,                                // bInterval: ignored for bulk transfer
        

        /* end CDC[0]*/
  
  /************** Descriptor of CUSTOM HID interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  HID0_REPORT_INTERFACE,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: CUSTOM_HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of CUSTOM_HID *************************/
  /* 18 */
  0x09,         /*bLength: CUSTOM_HID Descriptor size*/
  CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
  0x01,         /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  USBD_CUSTOM_HID_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Custom HID endpoints ********************/
  /* 27 */
    0x07,
    0x05,
    0x83,
    0x03,
    0x40,
    0x00,
    0x01,
    
    0x07,
    0x05,
    0x03,
    0x03,
    0x40,
    0x00,
    0x01
} ;




/* USB CUSTOM_HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_Desc[USB_CUSTOM_HID_DESC_SIZ] __ALIGN_END =
{
  /* 18 */
  0x09,         /*bLength: CUSTOM_HID Descriptor size*/
  CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
  0x11,         /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  USBD_CUSTOM_HID_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/* CUSTOM_HID  */
#pragma data_alignment=4
USBD_CUSTOM_HID_HandleTypeDef   phid;   

static uint8_t  USBD_CUSTOM_HID_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
  uint8_t                           ret = 0;
  USBD_CUSTOM_HID_HandleTypeDef     *hhid;
  
  pdev->pClassData = (void *)&phid;
  
  if(pdev->pClassData == NULL)
  {
    ret = 1;
  }
  else
  {
    hhid = (USBD_CUSTOM_HID_HandleTypeDef*) pdev->pClassData;      
    hhid->state = CUSTOM_HID_IDLE;
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->Init();
  }
    
  USBD_CDC_Init (pdev, cfgidx);
    
  return ret;
}

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         DeInitialize the CUSTOM_HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_HID_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{  
  if(pdev->pClassData != NULL)
  {
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->DeInit();
    pdev->pClassData = NULL;
  }
  return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_Setup
  *         Handle the CUSTOM_HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */

static uint8_t  USBD_CUSTOM_HID_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  USBD_CUSTOM_HID_HandleTypeDef     *hhid = (USBD_CUSTOM_HID_HandleTypeDef*)pdev->pClassData;

  if  (((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE && req->wIndex == HID0_REPORT_INTERFACE) ||
		((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_ENDPOINT && ((req->wIndex & 0x7F) == 2)))
	{

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
      
      
    case CUSTOM_HID_REQ_SET_PROTOCOL:
      hhid->Protocol = (uint8_t)(req->wValue);
      break;
      
    case CUSTOM_HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&hhid->Protocol,
                        1);    
      break;
      
    case CUSTOM_HID_REQ_SET_IDLE:
      hhid->IdleState = (uint8_t)(req->wValue >> 8);
      break;
      
    case CUSTOM_HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&hhid->IdleState,
                        1);
      break;
    
    case CUSTOM_HID_REQ_SET_REPORT:
      hhid->IsReportAvailable = 1;
      USBD_CtlPrepareRx (pdev, hhid->Report_buf, (uint8_t)(req->wLength));
      break;
      
    case CUSTOM_HID_REQ_GET_REPORT:
      hhid->IsReportAvailable = 1;
      USBD_CtlSendData (pdev, hhid->Report_buf, (uint8_t)(req->wLength));            
      break;
      
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( req->wValue >> 8 == CUSTOM_HID_REPORT_DESC)
      {
        len = MIN(USBD_CUSTOM_HID_REPORT_DESC_SIZE , req->wLength);
        pbuf =  ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->pReport;
      }
      else if( req->wValue >> 8 == CUSTOM_HID_DESCRIPTOR_TYPE)
      {
        pbuf = USBD_CUSTOM_HID_Desc;   
        len = MIN(USB_CUSTOM_HID_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&hhid->AltSetting,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      hhid->AltSetting = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}
  else
  		return USBD_CDC_Setup(pdev, req);

}

/**
  * @brief  USBD_CUSTOM_HID_SendReport 
  *         Send CUSTOM_HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
/*
uint8_t USBD_CUSTOM_HID_SendReport     (USBD_HandleTypeDef  *pdev, 
                                 uint8_t *report,
                                 uint16_t len)
{
  USBD_CUSTOM_HID_HandleTypeDef     *hhid = (USBD_CUSTOM_HID_HandleTypeDef*)pdev->pClassData;
  
  if (pdev->dev_state == USBD_STATE_CONFIGURED )
  {
    if(hhid->state == CUSTOM_HID_IDLE)
    {
      hhid->state = CUSTOM_HID_BUSY;
      USBD_LL_Transmit (pdev, 
                        CUSTOM_HID_EPIN_ADDR,                                      
                        report,
                        len);
    }
  }
  return USBD_OK;
}
*/

/**
  * @brief  USBD_CUSTOM_HID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CUSTOM_HID_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_CUSTOM_HID_CfgDesc);
  return USBD_CUSTOM_HID_CfgDesc;
}

/**
  * @brief  USBD_CUSTOM_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_CUSTOM_HID_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_CUSTOM_HID_HandleTypeDef     *hhid = (USBD_CUSTOM_HID_HandleTypeDef*)pdev->pClassData;  

  if (hhid->IsReportAvailable == 1)
  {
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->OutEvent(hhid->Report_buf);
    hhid->IsReportAvailable = 0;      
  }

  return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_CUSTOM_HID_EP0_TxSent(USBD_HandleTypeDef *pdev)
{
  USBD_CUSTOM_HID_HandleTypeDef     *hhid = (USBD_CUSTOM_HID_HandleTypeDef*)pdev->pClassData;  

  if (hhid->IsReportAvailable == 1)
  {
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->InEvent();
    hhid->IsReportAvailable = 0;      
  }

  return USBD_OK;
}

/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_CUSTOM_HID_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_CUSTOM_HID_DeviceQualifierDesc);
  return USBD_CUSTOM_HID_DeviceQualifierDesc;
}

/**
* @brief  USBD_CUSTOM_HID_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CUSTOMHID Interface callback
  * @retval status
  */
uint8_t  USBD_CUSTOM_HID_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                             USBD_CUSTOM_HID_ItfTypeDef *fops)
{
  uint8_t  ret = USBD_FAIL;
  
  if(fops != NULL)
  {
    pdev->pUserData= fops;
    ret = USBD_OK;    
  }
  
  return ret;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
