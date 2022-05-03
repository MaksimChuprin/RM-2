#ifndef DEVICE_STRUCT_H
  #define DEVICE_STRUCT_H

typedef int SOCKTYPE;

// structures, union ect
typedef struct {  
  Int32U  reiniuart1:         1;
  Int32U  reiniuart2:         1;
  Int32U  start_boot:         1;
  Int32U  reset_device:       1;  
  Int32U  wifi_on:            1;
  Int32U  reiniwifi:          1;
  Int32U  ethernet_on:        1;
  Int32U  usb_on:             1;  
  Int32U  Need2Save:          1;
  Int32U  NoInitNeed:         1;
  Int32U  gsm_on:             1;
  Int32U  mqtt_on:            1;
  Int32U  mqtt_ini:           1;  
  Int32U  hotreboot_ini:      1;
} system_flags_t;

typedef struct 
{  
  Int32U  timeinvalid:        1;
  Int32U  config_fail:        1;
  Int32U  dummy:              1;
  Int32U  wifi_fail:          1;
  Int32U  radio_fail:         1;
  Int32U  currentMemEnd:      1;  
  Int32U  savedMemEnd:        1;  
  Int32U  LSEfail:            1;
  Int32U  TimeSyncReq:        1;
  Int32U  gsm_fail:           1;  
  Int32U  NTPSyncReq:         1;  
} errors_flags_t;

#pragma pack(4)
typedef struct  // 16
{
  Int32U IP;
  Int32U Mask;
  Int32U Gate;
  Int8U  MAC[3];
  Int8U  UseDHCP:  1;
  Int8U  ConSpeed: 2;     // 0 - auto, 1 - 100, 2 - 10
  Int8U  ConFHDuplex: 1;  // 0 - full, 1 - half
} TCPIP_CONFIG;
#pragma pack()

#pragma pack(4)
typedef struct   // 160 bytes
{
  TCPIP_CONFIG WIFI_TCPIP;
  Int8U  NetName[64];
  Int8U  NetPass[64];
  Int8U  wifi_present:    1;
  Int8U  wifi_use:        1;
  Int8U  show_password:   1;
  Int8U  old_ver:         1;  
  Int8U  modbus:          1;
  Int8U  socket_timeout;
  Int16U ES_timeout;
  Int8U  Reserv[12];
} WIFI_CONFIG;
#pragma pack()

#pragma pack(4)
typedef struct   // 64 bytes
{
  TCPIP_CONFIG GSM_TCPIP;   // 16
  Int8U  APN[32];           // 32
  Int8U  gsm_present:    1; // 1
  Int8U  gsm_use:        1; // 0
  Int8U  err_code;          // 1
  Int8U  sms_mode;
  Int8U  Reserv[13];        // 14
} GSM_CONFIG;
#pragma pack()

#pragma pack(4)
typedef struct              // 96 bytes
{
  char   UserName[16];      // 16
  char   UserPass[16];      // 16
  char   TopicBase[16];     // 16
  char   SeverHostName[32]; // 32         
  U16    ServerPort;        // 2
  U16    mqtt_use:          1;
  U16    mqtt_interface:    2;  // 0 - Eth, 1 - GSM, 2 - WiFi
  U16    mqtt_buff_policy:  2;  // not used
  U16    mqtt_qos:          2;  // 0, 1, 2
  U16    ping_interval;     // 2
  U16    info_interval;     // 2
  U8     Reserv[8];         // 8
} MQTT_CONFIG;
#pragma pack()

#pragma pack(4)
typedef struct              // 48 bytes
{
  Int8U  NTPSeverName[32];  // 32
  S16    TimeZone;          // 2
  U16    ntpuse:       1;   // 2
  U32    periodUpdateTime;  // 4
  U32    periodValidTime;   // 4  
  U8     Reserv[4];         // 4
} NTP_CONFIG;
#pragma pack()

#if     defined(__V2XX__)
#pragma pack(4)
typedef struct  // 16
{   
  Int32U    Time;
  Int16U    KillTimer; 
  Int16S    Tempr;
  Int16S    Humidy;
  Int16U    Pressure;
  Int8U     PowLev;
  Int8U     Errors;
  Int8U     Adr;
  Int8U     RSSI:        2;
  Int8U     DATAREAD:    1;  
  Int8U     RECINUSE:    1;  
}  radio_list_struct_t;
#pragma pack()
#elif   defined(__V3XX__)
#pragma pack(4)
typedef struct  // 32
{   
  Int32U    Time;
  Int16S    RSSI;
  Int16U    Config;
  Int16U    Errors;
  Int16U    PowLev;
  
  Int16S    Tempr;
  Int16S    Humidy;
  Int16U    Pressure;
  Int16U    Measures[6];
  Int8U     Adr;
  Int8U     Type;  
}  radio_list_struct_t;
#else
  #error  "Version Not defined!"
#endif

#pragma pack(4)
typedef struct  // 20
{   
  Int32U    Time;
  Int32U    Total_Received_Count;
  Int32U    Packet_Error_Count;
  Int32U    Data_Error_Count;
  Int32U    RAM_Error_Count;
}  radio_statistic_struct_t;

#pragma pack(4)
typedef struct
{
  const U8 * data;
  U16 width;
  U16 height;
} tImage;
#pragma pack()

#pragma pack(4)
typedef struct
{
  long int code;
  const tImage *image;
} tChar;
#pragma pack()

#pragma pack(4)
typedef struct
{
  int length;
  const tChar *chars;
} tFont;
#pragma pack()

#pragma pack(2)
typedef struct
{
  Int16U        fColor;
  Int16U        bColor;  
  Int16U        Y; 
  const tFont   *fontType;
} TFT_String_st;
#pragma pack()

#endif