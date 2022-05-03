#include "arm_comm.h"

#define GSM_BUFLEN                        1024
#define GSM_RECIEVE_LEN                   256
#define GSM_TRANSMITT_LEN                 768
#define DEFAUL_GSM_TIMEOUT                250
#define GPRS_GSM_TIMEOUT                  30000

#define MQTT_CLIENT_GSM_SUPPORT           ENABLED
#define MQTT_CLIENT_TCP_SUPPORT           ENABLED

extern OS_TASK                            OS_GSM;

void GSM_Task                             (void);
void TIM4GSM_ISR                          (void);
void UART2GSM_IS                          (void);

void gsmSocketClose                       ( void );
U8   gsmsocketShutdown                    ( U32 timeout );
U8   gsmSocketConnect                     ( U32 ipAddr, U16 serverPort, U32 timeout );
U8   gsmOpenGPRS                          ( U32 timeout );
U8   gsmGetTimestamp                      ( pU32 timestamp, U32 timeout);
U8   gsmsocketSend                        ( const void * data, size_t length, size_t * written );
U8   gsmSocketReceive                     ( void * data, size_t size, size_t * received);
U8   gsmWaitForEvents                     ( U32 timeout );

#define GSM_RESET(A)                      { if((A)) GPIOD_BSRR  = (1<<7); else GPIOD_BRR   = (1<<7); }
