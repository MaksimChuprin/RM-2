#define USB_BULK_ASK            0x00
#define USB_BULK_CMDERR         0x01
#define USB_BULK_NAK            0xFF
#define BULK_SIZE               1024
#define MAX_DATA_LEN_USB        512
#define USB_DISCONNECT          { GPIOA_CRH &= 0x0fffffff; GPIOA_CRH |= 0x40000000; }
#define USB_CONNECT             { GPIOA_CRH &= 0x0fffffff; GPIOA_CRH |= 0x20000000; } 
#define USB_IFCONNECT           (GPIOA_IDR & BIT9)
#define WDATA_POINTER           &usbBuffer[8]
#define RDATA_POINTER           &usbBuffer[4]
#define ASK_USBANS              usbBuffer[0]
#define USB_IFCONNECT           (GPIOA_IDR & BIT9)

#define WR_USB                  0x00 
#define WF_USB                  0x01
#define WI_USB                  0x02

#define RR_USB                  0x80
#define RF_USB                  0x81
#define RI_USB                  0x82
#define IR_USB                  0x8F

#define BS_USB                  0x55
#define DW_USB                  0x56
#define DR_USB                  0x57

#define CT_USB                  0x77
#define RS_USB                  0x78
#define SS_USB                  0x79

extern OS_TASK                  OS_USB;
extern OS_STACKPTR U32          StackUSB[128];
extern U8                       usbBuffer[BULK_SIZE];

void                            USB_ISR_Handler(void);
void                            USB_Task(void);