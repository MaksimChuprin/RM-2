#include "defines.h"

/* Ivents */
OS_EVENT                          Uart2TXComplete;
OS_EVENT                          Uart2RXComplete;

/* Semafores */

/* mail boxes */
// OS_MAILBOX

/* таймера */

// Appl
volatile system_flags_t           system_flags;
U32                               OS_JLINKMEM_BufferSize = 128;
U32                               RadioChannel;
U32                               TimeSync;
volatile U16                      Rx2Counter;
volatile U16                      BytesToTransfer2;
U8                                gsm_buffer[GSM_BUFLEN];
pU8                               gsmReceiveBuf = &gsm_buffer[768];
pU8                               gsmSendBuf    = &gsm_buffer[0];
char                              HostName[32];

// конфигурация и озу

// CONFIG segment
__root __no_init U32              RsAdr            @ CONFIG(0); 
__root __no_init U32              RsSpeed          @ CONFIG(4);
__root __no_init U16              KillTime         @ CONFIG(8);
__root __no_init U8               RadioChannelModem     @ CONFIG(10);        
__root __no_init U8               RadioChannelRetrans   @ CONFIG(11);
__root __no_init U8               RadioPowerRetrans     @ CONFIG(12);
__root __no_init U8               RadioBW          @ CONFIG(13);
__root __no_init U8               ModemMaxTime     @ CONFIG(14);
__root __no_init U8               ReTransMaxTime   @ CONFIG(15);
__root __no_init U8               SerialNumber[15] @ CONFIG(16);
__root __no_init U8               ProtocolNumber   @ CONFIG(31);
__root __no_init TCPIP_CONFIG     TCPIPConfig      @ CONFIG(32);
__root __no_init WIFI_CONFIG      WiFiConfig       @ CONFIG(48);
__root __no_init U32              ConfigFlags      @ CONFIG(248);
__root __no_init U32              HotRebootTime    @ CONFIG(252);
__root __no_init GSM_CONFIG       GSMConfig        @ CONFIG(256);
__root __no_init MQTT_CONFIG      MQTTConfig       @ CONFIG(320);
__root __no_init NTP_CONFIG       NTPConfig        @ CONFIG(416);

// RAM segment
__root __no_init volatile errors_flags_t  errors_flags @ RAM(0);
__root __no_init U32              Time             @ RAM(4);
__root __no_init U32              CurrentCounter   @ RAM(8);
__root __no_init U32              SavedCounter     @ RAM(12);

__root __no_init radio_list_struct_t      CurrentList[MAX_CURRENT_POS] @ RAM(16);
__root __no_init radio_list_struct_t      SavedList  [MAX_SAVED_POS]   @ RAM(16 + MAX_CURRENT_POS * sizeof(radio_list_struct_t));
// __root __no_init radio_statistic_struct_t RadioDebug                   @ RAM(16 + (MAX_CURRENT_POS + MAX_SAVED_POS) * sizeof(radio_list_struct_t));

// constants
__root const U8                   ID[]     = "Радиомодем РМ-2-L ", Version[] = "r3.15";
/*
08/08/19 3.01 - управление буфером по модбас 
31/10/19 3.03 - обработка множественных запросов по modbusTCP, автоперезапуск по времени, проверка границ по протоколу №1
14/11/19 3.04 - проверка ОЗУ на валидность данных, фильтр метки времени
18/11/19 3.05 - возможность шума по температуре и влажности на основе rand
20/01/20 3.06 - модбас через WiFi
09/05/20 3.07 - анализатор пакетов для отладки, допконтрольная сумма на данные, перезапуск трансивера при ошибках
20/05/20 3.08 - перезапуск трансивера по времени
10/06/20 3.09 - аварийная
23/06/20 3.10 - SPI 2,25 МГц, баг проверки канала приема, shift_byte_spi1 - RXNE
06/07/20 3.11 - баг по первому протоколу - не обновлял CurrentBuffer
20/11/20 3.12 - CRC16 на ответы
30/11/20 3.14 - GSM, MQTT
21/04/21 3.15 - Modbus semafores, SSD1305/1309
30/12/21 3.16 - Web server
29/12/21 3.16 - USB GSM modem
*/
