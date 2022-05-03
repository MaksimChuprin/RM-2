#include "defines.h"

static  U8   SendMessWithAns      (U8 *msg, U8 *ans, U32 timeOut);
static  U32  send_data            (pU8 buffer, U32 len);
U16  send_message         (pU8 buffer);
U8   listenToModem        (U32 timeOut);
static  U8   GSM_STARTandCHECK    (void);

// static  char serverIP[]           = "111.222.333.444";

#pragma optimize = none
void  GSM_Task(void)
{
  uart_speed_stuct  speedset;
      
  // инициализация UART2
  speedset =  select_speed( 115200, APB1);
  
  USART2_BRR=              speedset.speed; 
  USART2_CR1= USART2_CR2 = USART2_CR3= USART2_GTPR= 0;
  
  // инициализация TIM4
  TIM4_CR1   = 0;                   // 
  TIM4_CCMR1 = BIT4;                // CC1 
  TIM4_DIER  = BIT1;                // CC1IE
  
  TIM4_ARR   =   TIM4_CCR1 = 6000;  // 
  TIM4_PSC   =   255;               // 72 000 000 / 256 - 20 мс таймаут между символами

  // инициализация в VIC  
  OS_ARM_InstallISRHandler(NVIC_USART2, (OS_ISR_HANDLER*)UART2_ISR);
  OS_ARM_ISRSetPrio       (NVIC_USART2, 0xFE);
  OS_ARM_EnableISR        (NVIC_USART2);
    
  OS_ARM_InstallISRHandler(NVIC_TIM4, (OS_ISR_HANDLER*)TIM4_ISR);
  OS_ARM_ISRSetPrio       (NVIC_TIM4, 0xFE);
  OS_ARM_EnableISR        (NVIC_TIM4);
  
  GSMConfig.err_code    =  GSM_STARTandCHECK();
  errors_flags.gsm_fail = (GSMConfig.err_code == 1);
  if( errors_flags.gsm_fail ) OS_Suspend(NULL);
    
  for(;;)
  {
    while( !GSMConfig.gsm_use )
    {      
      USART2_CR1_bit.RE   = 0;
      system_flags.gsm_on = 0;
      GSM_RESET(0);
      OS_WaitSingleEvent( BIT7 );
    }
    
    if( !system_flags.gsm_on )
    {
      GSMConfig.err_code = GSM_STARTandCHECK();
      switch( GSMConfig.err_code )
      {
        case 0: break;                      // good
        case 1: OS_Suspend(NULL);           // fatal error
                
        default: OS_Delay(10000); continue; //  SIM, NET - 60 snd
      }
    }
    
    // событие генерится при ошибке или выключении GSM  
    OS_WaitSingleEvent( BIT7 );
    system_flags.gsm_on = 0;
  }
}

// вкл модема
#pragma optimize = none
static U8 GSM_STARTandCHECK(void)
{  
  U8    r;
  
  USART2_CR1= USART2_CR2 = USART2_CR3= USART2_GTPR= USART2_SR = 0;
  
  GSM_RESET(0);
  OS_Delay(250); // To unconditionally reboot the GL868, the pad RESET* must be tied low for at least 200 milliseconds and then released
  GSM_RESET(1);
  OS_Delay(5000); // 4 s for power below 3.4 V
  
  OS_EVENT_Reset( &Uart2TXComplete); OS_EVENT_Reset( &Uart2RXComplete );  

  USART2_CR1_bit.UE =      1;
  USART2_CR1_bit.RE =      0;
  USART2_CR1_bit.TE =      1;
  USART2_CR1_bit.RXNEIE =  1;  
                  
  for(U8 i = 0; i < 5; i++, OS_Delay(500) )
  {
    r = SendMessWithAns("ATE0\r\n","OK", DEFAUL_GSM_TIMEOUT); // откл эхо
    if( !r ) break;
  }  
  if( r ) return 1; // нет ответа от модема
  
  SendMessWithAns("AT&K=0\r\n","OK", DEFAUL_GSM_TIMEOUT);     //  disabling hardware flow control 
  SendMessWithAns("AT#SLED=2\r\n","OK", DEFAUL_GSM_TIMEOUT);  //  Enable the function STAT_LED
  
  r  = SendMessWithAns("AT#QSS?\r\n",",1", DEFAUL_GSM_TIMEOUT);    
  if( r ) return 2; // нет симкарты
  
  for(U8 i = 0; i < 60; i++, OS_Delay(1000) )
  {
    r  = SendMessWithAns("AT+CREG?\r\n",",1", DEFAUL_GSM_TIMEOUT);
    if( !r ) break;
  }
  if( r ) return 3; // нет сети
  
//  sprintf( gsmSendBuf, "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", GSMConfig.APN );
//  r = SendMessWithAns( gsmSendBuf, "OK", DEFAUL_GSM_TIMEOUT);  // permits to open a TCP/UDP connection
//  if( r ) return 4;
  
  SendMessWithAns( "AT+CGDCONT=1,\"IP\",\"\"\r\n", "OK", DEFAUL_GSM_TIMEOUT);  // permits to open a TCP/UDP connection       
  system_flags.gsm_on = 1;
  return 0;
}

// отправить строку buffer
#pragma optimize = none
U32 send_data(pU8 buffer, U32 len)
{     
  memcpy(gsmSendBuf, buffer, len);
  
  OS_Delay(55);   // <txTo>  (default 50 ms) AT#SCFG
  BytesToTransfer2      = len;
  USART2_CR1_bit.RE     = 0;
  USART2_CR1_bit.TCIE   = 1;
  OS_EVENT_Wait( &Uart2TXComplete );
  
  return len;
}

// отправить строку buffer
#pragma optimize = none
U16 send_message(pU8 buffer)
{     
  strcpy( gsmSendBuf, buffer );  
  U16 len = strlen(gsmSendBuf);
  
  OS_Delay(25); // recommended 20 ms for AT-command
  BytesToTransfer2      = len;
  USART2_CR1_bit.RE     = 0;
  USART2_CR1_bit.TCIE   = 1;
  OS_EVENT_Wait( &Uart2TXComplete );  
    
  return len;
}

// отправить строку msg, сравнить ответ ans
#pragma optimize = none
static U8  SendMessWithAns(U8 *msg, U8 *ans, U32 timeOut)
{  
  send_message(msg);
  U8 result         = OS_EVENT_WaitTimed( &Uart2RXComplete, timeOut );
  USART2_CR1_bit.RE = 0;
  
  if( !result )
  {  
    char * p = strstr(gsmReceiveBuf, ans);       
    if( p == NULL )             return  2;  // no  ans
    else                        return  0;  // success
  }
                                return  1;  // timeout
}

#pragma optimize = none
U8 listenToModem(U32 timeOut)
{   
  Rx2Counter            =   0;
  USART2_CR1_bit.RE     =   1;
  U8 result             =   OS_EVENT_WaitTimed( &Uart2RXComplete, timeOut );
  USART2_CR1_bit.RE     =   0;

  return result;
}

static U8 OpenGPRS(U32 timeout)
{
  U8  r;
  
  // проверка состояния GPRS
  if( SendMessWithAns("AT#GPRS?\r\n", "OK", DEFAUL_GSM_TIMEOUT) ) return 1;
  
  char * p = strstr(gsmReceiveBuf, "0");       
  if( p == NULL ) return 0; // уже подключен
  
  for(U32 i = 0; i < 2; i++ )
  {
    OS_Delay(5000);
    r = SendMessWithAns("AT#GPRS=1\r\n", "OK", timeout);
    if( !r ) break;
  }
   
  return r; 
}

U8 gsmGetTimestamp( pU32 timestamp, U32 timeout)
{ 
  // вкл GPRS  
  if( OpenGPRS(timeout) ) return 1;
  
  // запрос времени у NTP сервера
  sprintf( gsmSendBuf, "AT#NTP=\"%s\",123,0,10,0\r\n", NTPConfig.NTPSeverName );
  if( SendMessWithAns( gsmSendBuf, "OK", timeout) ) return 1;
  
  char * p = strstr(gsmReceiveBuf, "NTP:");       
  if( p == NULL )            
  {
    return 1;
  } 
  else  p += 5;
  
  U8    s, mi, h, d, m, y;
  sscanf( p, "%u/%u/%u,%u:%u:%u", &y, &m, &d, &h, &mi, &s);  // NTP: 20/12/26,11:31:26
  *timestamp = EncodeDate(s, mi, h, d, m, y + 2000);

  return 0;  
}

U8 gsmOpenGPRS(U32 timeout)
{
  if (GSMConfig.sms_mode == 1) return 0;
  // вкл GPRS  
  return OpenGPRS(timeout);  
}

U8 gsmSocketConnect( U32 ipAddr, U16 serverPort, U32 timeout)
{
  U8    r = 0;
    
  r = SendMessWithAns("AT#SKTTO=0\r\n", "OK", DEFAUL_GSM_TIMEOUT); 
  if( r ) { OS_SignalEvent( BIT7, &OS_GSM); return 1; }
  
  sprintf( gsmSendBuf, "AT#SKTD=0,%d,\"%s\",0\r\n", serverPort, MQTTConfig.SeverHostName );
  r = SendMessWithAns( gsmSendBuf, "CONNECT", timeout);
  if( r )
  { 
    return 1; 
  }
  
  return 0;  
}

U8 gsmsocketShutdown( U32 timeout )
{    
  return 0;
}

void gsmSocketClose( void )
{
  OS_SignalEvent( BIT7, &OS_GSM);
}

U8  gsmsocketSend( const void * data, size_t length, size_t * written )
{
  send_data( (pU8)data, (U32)length);
  *written = length;
  
  return 0;
}

U8 gsmSocketReceive( void * data, size_t size, size_t * received)
{
  if( size > Rx2Counter )  size = Rx2Counter;
  
  Rx2Counter -= size;
  memcpy( data, gsmReceiveBuf, size);
  memcpy( gsmReceiveBuf, &gsmReceiveBuf[size], Rx2Counter);
  *received   = size;
  
  return 0;
}

U8  gsmWaitForEvents( U32 timeout )
{
  U8  r;
  
  r = listenToModem(timeout);
  if( !r )
  {
    if( strstr(gsmReceiveBuf, "NO CARRIER") == NULL ) return 0x40;
    return 0;
  }
    
  return 0;
}
