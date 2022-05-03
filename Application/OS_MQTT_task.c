#include "defines.h"
#include "IP_init.h"
#include <string.h>
#include "core/net.h"
#include "core/bsd_socket.h"
#include "core/socket.h"
#include "sntp/sntp_client.h"
#include "mqtt/mqtt_client.h"
#include "mqtt/mqtt_client_packet.h"
#include "mqtt/mqtt_client_transport.h"
#include "mqtt/mqtt_client_misc.h"
#include "debug.h"

#define T_ERROR           (1<<0)
#define H_ERROR           (1<<1)
#define P_ERROR           (1<<2)
#define NOHUM_ERROR       (1<<6)
#define ISPRESS_ERROR     (1<<7)

static MqttClientContext RM2_mqtt_context;
static char              topic_buff   [32];
static char              message_buff [512];

static U32               messageFor7M4_1 ( void );
static U32               messageFor7M4   ( void );
static U32               messageForMAG7  ( void );
static void              NTPService      ( void );
static void              setTime         ( U32 timeUTC );

void MQTT_Task(void) 
{    
  IpAddr  ipaddr         = { 4, 0 };
  U32     ntpLastTimeTry =        0;
  U32     ttime, pingtime, infotime, len;  
  U8      ntpTry         = errors_flags.TimeSyncReq && NTPConfig.ntpuse;
    
  for(system_flags.mqtt_ini = 1; ; OS_Delay(1000))
  {    
    system_flags.mqtt_on = 0;
    
    /* синхронизация времени */
    if( ntpTry ) 
    {       
      NTPService();
      ntpTry         = 0;
      ntpLastTimeTry = OS_GetTime32();
      continue;
    }    
    ntpTry = NTPConfig.ntpuse && (errors_flags.TimeSyncReq || errors_flags.NTPSyncReq) && ((ntpLastTimeTry + 3600000) < OS_GetTime32());
    /* синхронизация времени */
    
    if( !MQTTConfig.mqtt_use ) continue;
          
    if( system_flags.mqtt_ini )
    {
      system_flags.mqtt_ini = 0;      
      infotime              = 0;
      
      mqttClientInit            ( &RM2_mqtt_context);
      mqttClientBindToInterface ( &RM2_mqtt_context, &netInterface[0]);
      mqttClientSetIdentifier   ( &RM2_mqtt_context, SerialNumber);
      mqttClientSetAuthInfo     ( &RM2_mqtt_context, MQTTConfig.UserName, MQTTConfig.UserPass);
    }
    
    // set protocol and check interface
    switch( MQTTConfig.mqtt_interface )
    {
      case MQTT_THR_ETH: 
              if( !system_flags.ethernet_on )  continue;        
              if( getHostByName(&netInterface[0], MQTTConfig.SeverHostName, &ipaddr, HOST_TYPE_IPV4 | HOST_NAME_RESOLVER_DNS) ) continue;  // 
              mqttClientSetTimeout          ( &RM2_mqtt_context, 20000);
              mqttClientSetTransportProtocol( &RM2_mqtt_context, MQTT_TRANSPORT_PROTOCOL_TCP);
              break;
              
      case MQTT_THR_GSM:
              if( !system_flags.gsm_on )       continue;
              mqttClientSetTimeout          ( &RM2_mqtt_context, 60000);
              mqttClientSetTransportProtocol( &RM2_mqtt_context, MQTT_TRANSPORT_GSM);
              break;

      default: continue;
    }
    
    // connect to broker
    if ( mqttClientConnect( &RM2_mqtt_context, &ipaddr, MQTTConfig.ServerPort, true) != NO_ERROR ) continue;
    
    system_flags.mqtt_on = 1;
    
    for(;;OS_Delay(250))
    {
      //  Time Need Sync
      ntpTry = NTPConfig.ntpuse && (errors_flags.TimeSyncReq || errors_flags.NTPSyncReq) && ((ntpLastTimeTry + 3600000) < OS_GetTime32());
      if( ntpTry ) break; 
      
      // changes?
      if( system_flags.mqtt_ini ) break;
      
      // publish info-block
      ttime = OS_GetTime32();
      if( MQTTConfig.info_interval || !infotime )  
      {
        if( ((infotime + MQTTConfig.info_interval*1000) < ttime ) || !infotime )
        {
          sprintf( topic_buff, "%s/%s/info", MQTTConfig.TopicBase, SerialNumber );
          len = 0;
          message_buff[len++] = '{';
          len += sprintf( &message_buff[len], "   \"timeStamp\": %u ", Time);
          len += sprintf( &message_buff[len], " , \"deviceClass\": 71 ");
          len += sprintf( &message_buff[len], " , \"deviceName\": \"\xD0\xA0\xD0\x9C-2-L\" ");
          len += sprintf( &message_buff[len], " , \"firmwareVersion\": \"%s\" ", Version);
          len += sprintf( &message_buff[len], " , \"band\": %1u ", RadioChannelModem);
          len += sprintf( &message_buff[len], " , \"speed\": %1u ", (RadioBW & 3) + 1);
          len += sprintf( &message_buff[len], " , \"protocol\": %1u ", ProtocolNumber + 1);
          len += sprintf( &message_buff[len], " , \"deviceOnAir\": %u ", CurrentCounter);
          
          if( errors_flags.timeinvalid || errors_flags.radio_fail || errors_flags.TimeSyncReq )
          {
            len += sprintf( &message_buff[len], " , \"error\": ");
            message_buff[len++] = '[';
            char  commaCond = ' ';
            if(errors_flags.timeinvalid) { len += sprintf( &message_buff[len], "%c\"BadTime\" ",            commaCond); commaCond = ','; }
            if(errors_flags.TimeSyncReq) { len += sprintf( &message_buff[len], "%c\"NeedTimeSync\" ",       commaCond); commaCond = ','; }
            if(errors_flags.radio_fail)  { len += sprintf( &message_buff[len], "%c\"LoRaTransiverFail\" " , commaCond); commaCond = ','; }
            message_buff[len++] = ']';
          }
          
          message_buff[len++] = '}';
          if( mqttClientPublish( &RM2_mqtt_context, topic_buff, message_buff, len, MQTTConfig.mqtt_qos, true, NULL) != NO_ERROR ) break;  // break if anyerrors
          
          // сбросить время "неактивности" и infotime
          RM2_mqtt_context.pingTimestamp = infotime = OS_GetTime32();
        }
      }      
      
      // keepalive ping
      ttime = OS_GetTime32();
      if( MQTTConfig.ping_interval )  
      {
        if( (RM2_mqtt_context.pingTimestamp + MQTTConfig.ping_interval*1000) < ttime )
        {
          if( mqttClientPing( &RM2_mqtt_context, &pingtime) != NO_ERROR ) break; // break if anyerrors
        }
      }
                                  
      if( !SavedCounter ) continue; // нет данных для отправки
      
      sprintf( topic_buff, "%s/%s/data/%u", MQTTConfig.TopicBase, SerialNumber, SavedList[0].Adr );
      switch( SavedList[0].Type )
      {
        case IVTM_7M4_OKW:  len = messageFor7M4_1 ( );
                            break;
                            
        case IVTM_7M4_PRT:  len = messageFor7M4   ( );
                            break;
                                                
        case MAG_7:         len = messageForMAG7  ( ); 
                            break;
                                                
        default:            len = 0;
      }    
      if( mqttClientPublish( &RM2_mqtt_context, topic_buff, message_buff, len, MQTTConfig.mqtt_qos, true, NULL) != NO_ERROR ) break; // break if anyerrors

      // сбросить время "неактивности" 
      RM2_mqtt_context.pingTimestamp = OS_GetTime32(); 

      // обработка буффера SavedList
      OS_Use( &SemaRAM);
        SavedCounter--;
        memcpy((void *) &SavedList[0],(void *) &SavedList[1], (SavedCounter) * sizeof(radio_list_struct_t) ); // сместить буффер на одну запись
        memset((void *) &SavedList[SavedCounter], 0, sizeof(radio_list_struct_t) );                           // стереть последнюю запись
      OS_Unuse( &SemaRAM);
    }      
    
    mqttClientClose(&RM2_mqtt_context); // закрыть сокет при ошибке    
  }
}

static U32 messageFor7M4_1( void )
{  
  U32 len = 0;
    
  message_buff[len++] = '{';
  
  len += sprintf( &message_buff[len], "   \"timestamp\": %u ", SavedList[0].Time ); 
  len += sprintf( &message_buff[len], " , \"deviceClass\": 72 " );
  len += sprintf( &message_buff[len], " , \"deviceName\": \"\xD0\x98\xD0\x92\xD0\xA2\xD0\x9C-7\xD0\x9C\x34-1\" ");
  len += sprintf( &message_buff[len], " , \"channels\": ");
    message_buff[len++] = '[';
      message_buff[len++] = '{';
  
  len += sprintf( &message_buff[len], "   \"parameters\": ");
        message_buff[len++] = '[';
        
          message_buff[len++] = '{';          
  len += sprintf( &message_buff[len], "   \"value\": %.1f", SavedList[0].Tempr / 10.);
  len += sprintf( &message_buff[len], " , \"name\": \"\xD0\xA2\xD0\xB5\xD0\xBC\xD0\xBF\xD0\xB5\xD1\x80\xD0\xB0\xD1\x82\xD1\x83\xD1\x80\xD0\xB0\" ");
  len += sprintf( &message_buff[len], " , \"units\": \"\xC2\xB0\x43\" ");  
  if( SavedList[0].Errors & T_ERROR ) len += sprintf( &message_buff[len], ", \"error\": [\"\"] " );
          message_buff[len++] = '}';
  
  if( SavedList[0].Errors & NOHUM_ERROR ) ;
  else
  {
          message_buff[len++] = ',';
          message_buff[len++] = '{';
    len += sprintf( &message_buff[len], "   \"value\": %.1f", SavedList[0].Humidy / 10.);
    len += sprintf( &message_buff[len], " , \"name\": \"\xD0\x92\xD0\xBB\xD0\xB0\xD0\xB6\xD0\xBD\xD0\xBE\xD1\x81\xD1\x82\xD1\x8C\" ");
    len += sprintf( &message_buff[len], " , \"units\": \"%\" ");      
    if( SavedList[0].Errors & H_ERROR ) len += sprintf( &message_buff[len], " , \"error\": [\"\"] " );
          message_buff[len++] = '}';
  }
  
  if( SavedList[0].Errors & ISPRESS_ERROR )
  {
          message_buff[len++] = ',';
          message_buff[len++] = '{';
    len += sprintf( &message_buff[len], "   \"value\": %u", SavedList[0].Pressure);
    len += sprintf( &message_buff[len], " , \"name\": \"\xD0\x94\xD0\xB0\xD0\xB2\xD0\xBB\xD0\xB5\xD0\xBD\xD0\xB8\xD0\xB5\" ");
    len += sprintf( &message_buff[len], " , \"units\": \"\xD0\xBC\xD0\xBC.\xD1\x80\xD1\x82.\xD1\x81\xD1\x82.\" ");      
    if( SavedList[0].Errors & P_ERROR ) len += sprintf( &message_buff[len], " , \"error\": [\"\"] " );
          message_buff[len++] = '}';
  }  
  
          message_buff[len++] = ',';
          message_buff[len++] = '{';
  len += sprintf( &message_buff[len], "   \"value\": %u", SavedList[0].PowLev);
  len += sprintf( &message_buff[len], " , \"name\": \"\xD0\x97\xD0\xB0\xD1\x80\xD1\x8F\xD0\xB4\" ");
  len += sprintf( &message_buff[len], " , \"units\": \"%\" ");        
          message_buff[len++] = '}';

          message_buff[len++] = ',';
          message_buff[len++] = '{';
  len += sprintf( &message_buff[len], "   \"value\": %d", SavedList[0].RSSI);
  len += sprintf( &message_buff[len], " , \"name\": \"\xD0\xA1\xD0\xB8\xD0\xB3\xD0\xBD\xD0\xB0\xD0\xBB\" ");
  len += sprintf( &message_buff[len], " , \"units\": \"\xD0\xB4\xD0\x91\xD0\xBC\" ");        
          message_buff[len++] = '}';  
  
        message_buff[len++] = ']';  // parameters array
      message_buff[len++] = '}';
    message_buff[len++] = ']';  // channels array
    
  message_buff[len++] = '}';
  
  return len;
}

static U32 messageFor7M4( void )
{
  U32 len = 0;
    
  message_buff[len++] = '{';
  
  len += sprintf( &message_buff[len], "   \"timestamp\": %u ", SavedList[0].Time ); 
  len += sprintf( &message_buff[len], " , \"deviceClass\": 91 " );
  len += sprintf( &message_buff[len], " , \"deviceName\": \"\xD0\x98\xD0\x92\xD0\xA2\xD0\x9C-7\xD0\x9C\x34\" ");
  len += sprintf( &message_buff[len], " , \"channels\": ");
    message_buff[len++] = '[';
      message_buff[len++] = '{';
  
  len += sprintf( &message_buff[len], "   \"parameters\": ");
        message_buff[len++] = '[';
        
          message_buff[len++] = '{';          
  len += sprintf( &message_buff[len], "   \"value\": %.1f", SavedList[0].Tempr / 10.);
  len += sprintf( &message_buff[len], " , \"name\": \"\xD0\xA2\xD0\xB5\xD0\xBC\xD0\xBF\xD0\xB5\xD1\x80\xD0\xB0\xD1\x82\xD1\x83\xD1\x80\xD0\xB0\" ");
  len += sprintf( &message_buff[len], " , \"units\": \"\xC2\xB0\x43\" ");  
  if( SavedList[0].Errors & T_ERROR ) len += sprintf( &message_buff[len], ", \"error\": [\"\"] " );
          message_buff[len++] = '}';
  
  if( SavedList[0].Errors & NOHUM_ERROR ) ;
  else
  {
          message_buff[len++] = ',';
          message_buff[len++] = '{';
    len += sprintf( &message_buff[len], "   \"value\": %.1f", SavedList[0].Humidy / 10.);
    len += sprintf( &message_buff[len], " , \"name\": \"\xD0\x92\xD0\xBB\xD0\xB0\xD0\xB6\xD0\xBD\xD0\xBE\xD1\x81\xD1\x82\xD1\x8C\" ");
    len += sprintf( &message_buff[len], " , \"units\": \"%\" ");      
    if( SavedList[0].Errors & H_ERROR ) len += sprintf( &message_buff[len], " , \"error\": [\"\"] " );
          message_buff[len++] = '}';
  }
  
  if( SavedList[0].Errors & ISPRESS_ERROR )
  {
          message_buff[len++] = ',';
          message_buff[len++] = '{';
    len += sprintf( &message_buff[len], "   \"value\": %u", SavedList[0].Pressure);
    len += sprintf( &message_buff[len], " , \"name\": \"\xD0\x94\xD0\xB0\xD0\xB2\xD0\xBB\xD0\xB5\xD0\xBD\xD0\xB8\xD0\xB5\" ");
    len += sprintf( &message_buff[len], " , \"units\": \"\xD0\xBC\xD0\xBC.\xD1\x80\xD1\x82.\xD1\x81\xD1\x82.\" ");      
    if( SavedList[0].Errors & P_ERROR ) len += sprintf( &message_buff[len], " , \"error\": [\"\"] " );
          message_buff[len++] = '}';
  }  
  
          message_buff[len++] = ',';
          message_buff[len++] = '{';
  len += sprintf( &message_buff[len], "   \"value\": %u", SavedList[0].PowLev);
  len += sprintf( &message_buff[len], " , \"name\": \"\xD0\x97\xD0\xB0\xD1\x80\xD1\x8F\xD0\xB4\" ");
  len += sprintf( &message_buff[len], " , \"units\": \"%\" ");        
          message_buff[len++] = '}';

          message_buff[len++] = ',';
          message_buff[len++] = '{';
  len += sprintf( &message_buff[len], "   \"value\": %d", SavedList[0].RSSI);
  len += sprintf( &message_buff[len], " , \"name\": \"\xD0\xA1\xD0\xB8\xD0\xB3\xD0\xBD\xD0\xB0\xD0\xBB\" ");
  len += sprintf( &message_buff[len], " , \"units\": \"\xD0\xB4\xD0\x91\xD0\xBC\" ");        
          message_buff[len++] = '}';  
  
        message_buff[len++] = ']';  // parameters array
      message_buff[len++] = '}';
    message_buff[len++] = ']';  // channels array
    
  message_buff[len++] = '}';
  
  return len;
}
                 
static U32 messageForMAG7( void )
{
  return 0; 
}

static void NTPService( void)
{
  IpAddr        ipaddr    = { 4, 0 };
  NtpTimestamp  timestamp = { 0, 0 };
  U32           seconds   = 0;
  
  // канал ethernet
  if( system_flags.ethernet_on )
  {
    if( !getHostByName(&netInterface[0], NTPConfig.NTPSeverName, &ipaddr, HOST_TYPE_IPV4 | HOST_NAME_RESOLVER_DNS) ) 
    {
      if( !sntpClientGetTimestamp(&netInterface[0], &ipaddr, &timestamp) )  
      {
        setTime( timestamp.seconds - 0x83AA7E80 ); // 0x83AA7E80 = 70 лет от 01-01-1900 до 01-01-1970
        return;
      }      
    }
  }

  // канал gsm  
  if( !GSMConfig.gsm_present || !GSMConfig.gsm_use ) return;
  
  // ждем сеть
  for(U8 i = 0; ; i++, OS_Delay(1000))
  {    
    if( system_flags.gsm_on ) break;
    if( i > 60 )              return;
  }
 
  // запрос UNIX времени
  if( !gsmGetTimestamp( &seconds, 30000) )  setTime( seconds );
  
  // закрыть соединение
  gsmSocketClose();
  return;    
}

static void setTime ( U32 timeUTC )
{
  U32 timeshift = abs(3600 * NTPConfig.TimeZone);
        
  OS_Use( &SemaRAM);
    if( NTPConfig.TimeZone < 0 )  Time = timeUTC - timeshift;
    else                          Time = timeUTC + timeshift;
    
    RTC_SetCounter( Time );
                                
    set_bkp_reg( ERR_BASE, 0);
    set_bkp_reg( TIMESYNC_BASE,     Time & 0xffff);
    set_bkp_reg( TIMESYNC_BASE + 1, Time >> 16);
    
    errors_flags.NTPSyncReq = errors_flags.TimeSyncReq = errors_flags.timeinvalid = 0; 
  OS_Unuse( &SemaRAM);
}
