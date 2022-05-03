#include "modbus.h"

#define UDP_PORT                  1337
#define COMMAND_PORT              502
#define WEB_PORT                  80
#define SOCKET_TIMEOUT            1000
#define MAX_SOCKET_NUM            1

#define UDP_SAFE_MAXLEN           1024
#define UDP_BUF_LEN               1024 + 8
#define TCP_MODBUS_LEN            MAX_REG_COUNT * 2 + MODBUS_TCP_HEADER_LEN + MODBUS_CRC_LEN + MODBUS_RTU_HEADER_LEN

#define CURRENT_BUFFER_PAGE       0
#define ARCHIVE_BUFFER_PAGE       1
#define STATUS_PAGE               2

extern OS_TASK                    TCP80, TP502, TCP_CMD;
extern OS_STACKPTR U32            TCP80Stack[256], IP502Stack[256], TCPCMDStack[256];

SOCKTYPE                          ListenAtTcpAddr       (U16 port);
SOCKTYPE                          OpenAtUdpAddr         (U16 port);

void                              TCP80_Task                   (void);
void                              IP502_Task                   (void);
void                              UDP_Task                     (void);
U16                               EKSIS_WIFI_TCP               (pU8 tcp_buffer);
void                              ETH_IRQHandler               (void);

void proceedWEB                   (SOCKTYPE pOutput, pU8 buffer);
void WS_SendHead                  (SOCKTYPE pOutput, pU8 buffer);
void WS_SendCSS                   (SOCKTYPE pOutput);
void WS_SendInfo                  (SOCKTYPE pOutput, pU8 buffer);
void WS_SendLinks                 (SOCKTYPE pOutput, pU8 buffer, U8 selected);
void WS_SendCurrentPage           (SOCKTYPE pOutput, pU8 buffer);
void WS_SendArchivePage           (SOCKTYPE pOutput, pU8 buffer);
void WS_SendStatusPage            (SOCKTYPE pOutput, pU8 buffer);
void WS_SendRadioListStruct       (SOCKTYPE pOutput, pU8 buffer, radio_list_struct_t* rls);