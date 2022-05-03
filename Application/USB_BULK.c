#include "USB.h"
#include "defines.h"

// global
OS_TASK                           OS_USB;
OS_STACKPTR U32                   StackUSB[128];

#pragma data_alignment= 4
static U8                         usbBuffer[BULK_SIZE];

/* Add generic USB BULK interface to USB stack */
static void _AddBULK(void) {
  static U8 _abOutBuffer[USB_MAX_PACKET_SIZE];
  USB_BULK_INIT_DATA    InitData;

  InitData.EPIn  = USB_AddEP(1, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, NULL, 0);
  InitData.EPOut = USB_AddEP(0, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, _abOutBuffer, USB_MAX_PACKET_SIZE);
  USB_BULK_Add(&InitData);
}

/* USB_GetVendorId */
U16 USB_GetVendorId(void) {
  return 0x3412;
}

/* USB_GetProductId */
U16 USB_GetProductId(void) {
  return 0x1006;
}

/* USB_GetVendorName */
const char * USB_GetVendorName(void) {
  return "EKSIS";
}

/* USB_GetProductName */
const char * USB_GetProductName(void) {
  return "Bulk device";
}

/* USB_GetSerialNumber */
const char * USB_GetSerialNumber(void) {
  return (const char *)SerialNumber;
}

void USB_Task(void) 
{
  U8            command, check_sum = 1;
  U16           bytenum;
  U32           addr, i;  
   
  USB_Init();
  _AddBULK();
  USB_Start();
  GPIOA_CRH= 0x4222224B;
  USB_CONNECT; 
  
  for(;;) 
  {    
    // Wait for configuration
    while (!USB_IsConfigured()) 
    {
      OS_Delay(100);
    }

    if(USB_BULK_Read(usbBuffer, BULK_SIZE) != BULK_SIZE)  
    {
      OS_Delay(100);
      continue;
    }
    
    addr     =  *((pU32)usbBuffer);
    bytenum  =  *((pU16)&usbBuffer[4]);
    command  =  *((pU16)&usbBuffer[6]);
    bytenum &=  0x3ff;
    
    switch(command)
    {
      case WR_USB:
      case WI_USB:          
                    for (i = 0, check_sum = 0xff; i < (8 + bytenum); i++) check_sum += usbBuffer[i];
                    check_sum = (check_sum != usbBuffer[8 + bytenum]) ? 1 : 0;
                    break;
      
      case RR_USB:
      case RI_USB:
      case RF_USB:
      case IR_USB:
                    for (i = 0, check_sum = 0xff; i < 8; i++) check_sum += usbBuffer[i];
                    check_sum = (check_sum != usbBuffer[8]) ? 1 : 0;
                    break;
                    
      case BS_USB:  check_sum = 0;
    }
    
    if(check_sum) goto fail;
    
    switch(command)
    {
    // WR 
    case WR_USB:  if( (bytenum > MAX_DATA_LEN_USB) || ((addr + bytenum) > RAMSIZE) ) goto fail;
                  OS_Use( &SemaRAM);
                    binbuffer_2_binbuffer( WDATA_POINTER, (pU8)(RamBeginAddr + addr), bytenum);
                    // CLOCK
                    PWR_CR_bit.DBP=       1;
                    while(!RTC_CRL_bit.RTOFF);
                    RTC_CRL_bit.CNF=      1;
                    RTC_CNTL=             (U32)Time & 0xffff;
                    RTC_CNTH=             (U32)Time >> 16;
                    RTC_CRL_bit.CNF=      0;
                    while(!RTC_CRL_bit.RTOFF);
                    PWR_CR_bit.DBP=       0;
                    ProceedSaveList();
                  OS_Unuse( &SemaRAM);
                  ASK_USBANS=  USB_BULK_ASK;
                  
                  usbBuffer[8] = 0xFF;
                  for (i = 0; i < 8; i++) usbBuffer[8] += usbBuffer[i];
                  break;
                                                      
    // WI 
    case WI_USB:  if( (bytenum > MAX_DATA_LEN_USB) || ((addr + bytenum) > CONFIGSIZE) )  goto fail;
                  OS_Use( &SemaRAM);
                    binbuffer_2_binbuffer( WDATA_POINTER, (pInt8U)(ConfigBeginAddr + addr), bytenum);
 
                    // write flash                    
                    writeConfigFlashWithCHK();                    
                    system_flags.start_boot = 1;
                    
                  OS_Unuse( &SemaRAM);                                    
                                                      
                  ASK_USBANS   = USB_BULK_ASK;                  
                  usbBuffer[8] = 0xFF;
                  for (i = 0; i < 8; i++) usbBuffer[8] += usbBuffer[i];
                  break;
                  
    // RR
    case RR_USB:  if( (bytenum > MAX_DATA_LEN_USB) || ((addr + bytenum) > RAMSIZE) )  goto fail;
                  OS_Use( &SemaRAM);
                    binbuffer_2_binbuffer( (pInt8U)(RamBeginAddr + addr), RDATA_POINTER, bytenum);
                    set_flags_data_read(RamBeginAddr + addr, bytenum);
                  OS_Unuse( &SemaRAM);
                  ASK_USBANS = USB_BULK_ASK;
                  for (i = 0, usbBuffer[8 + bytenum] = 0xff; i < 8 + bytenum; i++) usbBuffer[8 + bytenum] += usbBuffer[i];
                  break;
                  
    // RF
    case RF_USB:  if( (bytenum > MAX_DATA_LEN_USB) )   goto fail;
                  binbuffer_2_binbuffer( (pU8)main_screen, RDATA_POINTER, bytenum);
                  ASK_USBANS = USB_BULK_ASK;
                  for (i = 0, usbBuffer[8 + bytenum] = 0xff; i < 8 + bytenum; i++) usbBuffer[8 + bytenum] += usbBuffer[i];
                  break;                  

    // RI         
    case RI_USB:  if( (bytenum > MAX_DATA_LEN_USB) || ((addr + bytenum) > CONFIGSIZE) )  goto fail;
                  binbuffer_2_binbuffer( (pInt8U)(ConfigBeginAddr + addr), RDATA_POINTER, bytenum);
                  ASK_USBANS = USB_BULK_ASK;
                  for (i = 0, usbBuffer[8 + bytenum] = 0xff; i < 8 + bytenum; i++) usbBuffer[8 + bytenum] += usbBuffer[i];                      
                  break;                  
    // IR
    case IR_USB:  bytenum = 8;
                  for( i = 0; i < 8; i++, bytenum++)         usbBuffer[bytenum] = SerialNumber[i];
                  usbBuffer[bytenum++]  = 0;
                  for(i= 0; Version[i]; i++, bytenum++)      usbBuffer[bytenum] = Version[i];
                  usbBuffer[bytenum++]  = 0;
                  for(i= 0; ID[i]; i++, bytenum++)           usbBuffer[bytenum] = ID[i];
                  usbBuffer[bytenum]    = 0;
                  bytenum++;
                  *((pU16)&usbBuffer[4]) = bytenum - 8;                  
                  ASK_USBANS= USB_BULK_ASK;
                  for (i = 0, usbBuffer[bytenum] = 0xff; i < bytenum; i++) usbBuffer[bytenum] += usbBuffer[i];
                  break;
                  
     // BS
     case BS_USB: ASK_USBANS = USB_BULK_ASK;
                  OS_Use( &SemaRAM);
                    system_flags.start_boot= 1;
                      OS_Unuse( &SemaRAM);
                  break;                                                             
     fail:
     default:     ASK_USBANS = USB_BULK_NAK;
    }
    USB_BULK_Write(usbBuffer, BULK_SIZE);  
    
    if(system_flags.start_boot) 
    {
      USB_DISCONNECT;
      RCC_AHBRSTR_bit.OTGFSRST = 1;
      OS_Delay(500);
      *((pU32)0x2000c000) = 0x12345678;
      HW_RESET;
    }
  }
}
