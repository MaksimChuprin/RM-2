extern OS_TASK                      OS_SUPERVISER, OS_WIFI; 
extern OS_RSEMA                     SemaRAM;
extern OS_TIMER                     KeyTimer, Config_Timer;
extern OS_STACKPTR U32              StackSUPERVISER[];

extern OS_EVENT                     Uart2TXComplete;
extern OS_EVENT                     Uart2RXComplete;

// global var
extern volatile system_flags_t      system_flags;
extern volatile errors_flags_t      errors_flags;

extern U32                          RadioChannel;
extern S16                          RSSI[128];
extern U16                          ConfigSaveTime, KeyCode;
extern volatile U16                 Rx2Counter, BytesToTransfer2;
extern U8                           MeasureDisplayPointer;
extern U8                           gsm_buffer[GSM_BUFLEN];
extern char                         HostName[];
extern pU8                          gsmReceiveBuf;
extern pU8                          gsmSendBuf;

// ram+rom
// CONFIG segment
extern U32                          RsSpeed, RsAdr, IDWordLo, HotRebootTime, ConfigFlags;
extern U16                          KillTime;
extern U8                           RadioChannelModem, RadioChannelRetrans, RadioPowerRetrans, RadioBW, ProtocolNumber, SerialNumber[], DeviceName[];
extern U8                           ModemMaxTime, ReTransMaxTime;

extern TCPIP_CONFIG                 TCPIPConfig;
extern WIFI_CONFIG                  WiFiConfig;
extern GSM_CONFIG                   GSMConfig;
extern MQTT_CONFIG                  MQTTConfig;
extern NTP_CONFIG                   NTPConfig;

// RAM segment
extern U32                          CurrentCounter, SavedCounter, Time;
extern radio_list_struct_t          CurrentList[MAX_CURRENT_POS], SavedList[MAX_SAVED_POS];
extern radio_statistic_struct_t     RadioDebug;

// consts
extern const U8                     ID[], Version[];
extern const U32                    IDWordHi;