#include "defines.h"

/* Semafores */

/* mail boxes */
// OS_MAILBOX

/* таймера */

// Appl
volatile system_flags_t           system_flags;
U32                               OS_JLINKMEM_BufferSize = 128;
U32                               RadioChannel;
S16                               RSSI[128];
char                              HostName[32];

// конфигурация и озу

// CONFIG segment
__root __no_init U32              RsAdr            @ CONFIG(0);
__root __no_init U32              RsSpeed          @ CONFIG(4);
__root __no_init U16              KillTime         @ CONFIG(8);
__root __no_init U8               RadioChannelModem   @ CONFIG(10); 
__root __no_init U8               RadioChannelRetrans @ CONFIG(11);
__root __no_init U8               RadioPowerRetrans   @ CONFIG(12);
__root __no_init U8               RadioBW          @ CONFIG(13);

__root __no_init U8               SerialNumber[16] @ CONFIG(16);
__root __no_init TCPIP_CONFIG     TCPIPConfig      @ CONFIG(32);
__root __no_init WIFI_CONFIG      WiFiConfig       @ CONFIG(48);
__root __no_init U32              ConfigFlags      @ CONFIG(248);
__root __no_init U32              HotRebootTime    @ CONFIG(252);
__root __no_init U32              IDWordLo         @ CONFIG(256);

// RAM segment
__root __no_init volatile errors_flags_t  errors_flags @ RAM(0);
__root __no_init U32              Time             @ RAM(4);
__root __no_init U32              CurrentCounter   @ RAM(8);
__root __no_init U32              SavedCounter     @ RAM(12);

__root __no_init radio_list_struct_t  CurrentList[128] @ RAM(16);
__root __no_init radio_list_struct_t  SavedList[1024]  @ RAM(2064);

// constants
__root const U8                   ID[]     = "Радиомодем РМ-2-L ", Version[] = "r2.04";
// 23.01.2018 - 2.01 Modbus TCP (RTU crc)
// 11.05.2018 - 2.02 NOHUM mask
// 13.07.2018 - 2.03 LSE хитрый запуск
// 08.07.2019 - 2.04 марафет
__root const U32                  IDWordHi = _EKSIS_UNIX_TIME;
U8               ProtocolNumber = 0;
