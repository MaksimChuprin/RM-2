#include "defines.h"

// local
static U8                            doNotReceive;
static const pU8                     wifiExchBuff = &gsm_buffer[64];

static U8                            wifi_initialization (void);
static U16                           send_message (pU8 buffer);
static U8                            SendMessWithAns (U8 *msg, U8 *ans, U32 timeout);
static U8                            get_my_ip (void);
static U8                            get_my_ip_NEW (void);
static U8                            checkWiFi (void);
static void                          send_data_WiFi(pU8 buffer, U8 connection, U16 len);
static U16                           recieve_data_WiFi(U32 timeout, pU8 connection, pU8 * datapoint);

#define SEND_UART_DEB(A)         { USART1_CR1_bit.RE = 0; BytesToTransfer1 = (A); USART1_CR1_bit.TCIE = 1; }

#ifdef ES8266_DEB 
static const U8  ansODOA[]        = { 0x0d, 0x0a, 0 };
#endif

static U8 usb_buf[128];
static volatile U8 usb_req;
static volatile U8 usb_len;
volatile U8 usb_start;
// isr TIM4
#pragma optimize = none
void  TIM4_ISR(void)
{
  OS_EnterInterrupt();
  
  TIM4_SR               &=  ~BIT1;
  TIM4_CR1              &=  ~BIT0;
  //USART2_CR1_bit.RE      =   0;  
  gsmReceiveBuf[Rx2Counter] = 0;
  OS_EVENT_Set  ( &Uart2RXComplete);  
  if (GSMConfig.sms_mode == 1) 
  {
    if (usb_start==1)
    {
      usb_len = Rx2Counter;
      memcpy(usb_buf, gsmReceiveBuf, Rx2Counter);
      TEMPLATE_Transmit(usb_buf, usb_len);
      usb_start = 0;
    }
    Rx2Counter = 0;
  }
  else 
    USART2_CR1_bit.RE      =   0;  
  
  OS_LeaveInterrupt();
}

// isr uart2
#pragma optimize = none
void UART2_ISR(void) 
{
  static U16    txCounter;
  U8            inByte;
  U32           uart_status;
  
  OS_EnterInterrupt();
  
  uart_status  = USART2_SR;
  inByte       = USART2_DR;  
  
  if(uart_status & (1<<5))  // RXNE
  {        
    if( Rx2Counter == GSM_RECIEVE_LEN )
    { 
      Rx2Counter    = 0; 
    }
    else  
    {
      gsmReceiveBuf[Rx2Counter++] = inByte;     
      //запустить или обнулить таймер
      TIM4_EGR     |=   BIT0;
      TIM4_CR1     |=   BIT0;
    }
  }
  
  if(USART2_CR1_bit.TCIE && (uart_status & (1<<6))) // TC
  {    
    if( (txCounter == BytesToTransfer2 ) || ( txCounter == GSM_TRANSMITT_LEN ))
    {
      BytesToTransfer2 = Rx2Counter  =  0;
      txCounter             =  0;
      USART2_CR1_bit.TCIE   =  0;
      USART2_SR_bit.RXNE    =  0;
      USART2_CR1_bit.RE     =  1;
      OS_EVENT_Reset  ( &Uart2RXComplete);
      OS_EVENT_Set    ( &Uart2TXComplete);
    } 
    else USART2_DR = gsmSendBuf[txCounter++];
  }
  
  OS_LeaveInterruptNoSwitch();
}

void  WIFI_Task(void)
{
  S16               dnum;
  U16               len;
  U8                connectionN;
  pU8               dpointer;
  uart_speed_stuct  speedset;
    
  // инициализаци€ UART2
  speedset =  select_speed( 115200, APB1);
  
  USART2_BRR=              speedset.speed; 
  USART2_CR1= USART2_CR2 = USART2_CR3= USART2_GTPR= USART2_SR = 0;
  
  // инициализаци€ TIM4
  TIM4_CR1   = 0;                   // 
  TIM4_CCMR1 = BIT4;                // CC1 
  TIM4_DIER  = BIT1;                // CC1IE
  
  TIM4_ARR   =   TIM4_CCR1 = 6000;  // 
  TIM4_PSC   =   255;               // 72 000 000 / 256 - 20 мс таймаут между символами

  
  // инициализаци€ в VIC   
  OS_ARM_InstallISRHandler(NVIC_USART2, (OS_ISR_HANDLER*) UART2_ISR);
  OS_ARM_ISRSetPrio(NVIC_USART2, 0xFE);
  OS_ARM_EnableISR(NVIC_USART2); 
  
  OS_ARM_InstallISRHandler(NVIC_TIM4, (OS_ISR_HANDLER*)    TIM4_ISR);
  OS_ARM_ISRSetPrio(NVIC_TIM4, 0xFE);
  OS_ARM_EnableISR(NVIC_TIM4);
  
  // инициализаци€
  errors_flags.wifi_fail = wifi_initialization();  
  if( errors_flags.wifi_fail ) OS_Suspend(NULL);
      
  for(;;)
  {     
    // проверка состо€ни€ модема и WiFi подключени€
    if( checkWiFi() ) 
    { 
      OS_Delay(2000); 
      continue; 
    }
    
    // прием данных
    for(U8 a = 1; a; )
    {
     dnum = recieve_data_WiFi(WiFiConfig.ES_timeout * 1000, &connectionN, &dpointer);
        
     switch( dnum) 
     {
      case 1:   // таймаут неактивности модема
                a = 0; 
                system_flags.reiniwifi = 1;                     // без обмена - сброс модема - 1-й подниматель п»нгвинов
                break;
              
      case 2:  // служебные сообщени€ модема
                if( system_flags.reiniwifi) a = 0;
                break;              
        
      default:  // прин€т пакет
                memcpy( (void *)wifiExchBuff, (void *)dpointer, dnum );
                // обработка запроса
               if( WiFiConfig.modbus) 
               {
                len = MODBUS_TCP((pU8)wifiExchBuff);
               }
               else                   
               {
                len = EKSIS_WIFI_TCP((pU8)wifiExchBuff);                
               }
               send_data_WiFi((pU8)wifiExchBuff, connectionN, len);
     } // switch
    } // for
  } // for
}

// инициализаци€ и настройка модема
#pragma optimize = none
static U8  wifi_initialization(void)
{ 
  uart_speed_stuct  speedset;
  
  USART2_CR1= USART2_CR2 = USART2_CR3= USART2_GTPR= USART2_SR = 0;

  WIFI_RESER_LOW;
  OS_Delay(200);
  WIFI_RESET_HIGH;
  OS_Delay(2000);
  
  OS_EVENT_Reset( &Uart2TXComplete); OS_EVENT_Reset( &Uart2RXComplete );
  
  USART2_CR1_bit.UE =      1;
  USART2_CR1_bit.RE =      0;
  USART2_CR1_bit.TE =      1;
  USART2_CR1_bit.RXNEIE =  1;

  // старые версии ES8266 с деф скоростью 9600
  speedset      =  select_speed( 115200, APB1);
  USART2_BRR    =  speedset.speed;
  
  if( SendMessWithAns("AT\r\n","OK", 200) )
  {
    // не ответил на 115200 - уже на 9600
    speedset    =  select_speed( 9600, APB1);
    USART2_BRR  =  speedset.speed;
    
    SendMessWithAns("AT\r\n","OK", 200);
    
    if( SendMessWithAns("AT\r\n","OK", 200) )  return 1;  // не ответил на 9600 - треш
    
    if( !SendMessWithAns("AT+UART_CUR=9600,8,1,0,0\r\n","OK", 400) ) WiFiConfig.old_ver = 0;
    else                                                             WiFiConfig.old_ver = 1;   // не поддерживает новые команды _CUR _DEF   - старый софт 
  }
  else
  {    
    // ответил на 115200 - мен€ем скорость на 9600
    if( !SendMessWithAns("AT+UART_CUR=9600,8,1,0,0\r\n","OK", 400) ) WiFiConfig.old_ver = 0;
    else
    {
      // не поддерживает новые команды _CUR _DEF  и не отвечает на команду перевода скорости  - старый софт
      SendMessWithAns("AT+UART=9600,8,1,0,0\r\n","OK", 400);
      WiFiConfig.old_ver = 1;
    }
    
    speedset    =  select_speed( 9600, APB1);
    USART2_BRR  =  speedset.speed;
    OS_Delay(100);
    
    SendMessWithAns("AT\r\n","OK", 200);                 // прогон
    if( SendMessWithAns("AT\r\n","OK", 200) ) return 1;  // не переводит скорость - треш    
  }
      
  SendMessWithAns("AT\r\n","OK", 200);
    SendMessWithAns("AT+CIPMUX=1\r\n","OK", 200);    // мультисоединени€
      SendMessWithAns("AT+CWMODE=3\r\n","OK", 200);  // AP + Station mode

  if( WiFiConfig.old_ver) sprintf(gsmSendBuf, "AT+CWJAP=\"%s%s%s%s%s\r\n", WiFiConfig.NetName, "\",", "\"", WiFiConfig.NetPass, "\""); 
  else                    sprintf(gsmSendBuf, "AT+CWJAP_CUR=\"%s%s%s%s%s\r\n", WiFiConfig.NetName, "\",", "\"", WiFiConfig.NetPass, "\""); 
  
  return SendMessWithAns(gsmSendBuf,"OK", 1000);    // Connect to AP   
}

// проверка состо€ни€ сети WIFi , управление состо€нием серверного сокета
U8  checkWiFi(void)
{
  static  U8  noconnCount;
  
  if( !WiFiConfig.wifi_use )
  {
    WIFI_RESER_LOW;
    system_flags.wifi_on   = noconnCount = 0;
    system_flags.reiniwifi = 1;
    return 1;
  }
      
  // отвечаем?
  U8 r = SendMessWithAns("AT\r\n","OK", 200);
  if( (r == 1) || system_flags.reiniwifi ) 
  {
    // савсэм плохой...              
    system_flags.wifi_on   = system_flags.reiniwifi = noconnCount = 0;
    errors_flags.wifi_fail = wifi_initialization();
    return 1;
  }
  
  // зан€т?
  if( r == 2 ) return 1;
  
  
  if( WiFiConfig.old_ver) sprintf(gsmSendBuf,"AT+CWJAP?\r\n");  
  else                    sprintf(gsmSendBuf,"AT+CWJAP_CUR?\r\n"); 
  
  // подключен?
  if( SendMessWithAns( gsmSendBuf, WiFiConfig.NetName, 500) )
  {
    // нет соединени€ с сетью
    if( noconnCount++ > 10)
    {
      system_flags.reiniwifi = 1;          
      noconnCount = system_flags.wifi_on = 0;
    }
    return 1;
  }
  
  noconnCount = 0;
  
  // открыть сервер 1 раз 
  if( !system_flags.wifi_on )
  {
    SendMessWithAns("AT+CIPSERVER=1,502\r\n", "OK", 500); // серверный порт 502
    sprintf(gsmSendBuf, "AT+CIPSTO=%d\r\n", WiFiConfig.socket_timeout);
    SendMessWithAns( gsmSendBuf, "OK", 500);
    
    if(WiFiConfig.old_ver)  return get_my_ip();
    else                    return get_my_ip_NEW();        
  }  
    
  return 0;
}

// получить розданный от AP IP
static U8 get_my_ip(void)
{
  U32  p1 = 0, b1, b2, b3, b4;
  
  if( SendMessWithAns("AT+CIFSR\r\n", "OK", 500) ) return 1;
  
  // ищем положение IP
  for(U8 i = 2; i < Rx2Counter - 8; i++)
    if( gsmReceiveBuf[i] == 0x0a )
    {
      p1              =   i;
      gsmReceiveBuf[Rx2Counter - 8]  = gsmReceiveBuf[i]  = ' ';
      break;
    }
  
  // "плоха€" строка
  if( !p1 )
  {
    return 1;
  }  
    
  sscanf( &gsmReceiveBuf[p1], "%u.%u.%u.%u", &b1, &b2, &b3, &b4);
  WiFiConfig.WIFI_TCPIP.IP = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
  system_flags.wifi_on     = 1;
  return 0;
}

// получить розданный от AP IP
static U8 get_my_ip_NEW(void)
{
  U32   b1, b2, b3, b4;
  U8    *p;
  
  if( SendMessWithAns( "AT+CIFSR\r\n", "OK", 500 ) ) return 1;
  
  // ищем положение IP
  p = strstr(gsmReceiveBuf,"STAIP");
  
  // "плоха€" строка
  if( p == NULL ) return 1;
  
  p[6] = ' ';
  sscanf( (U8 const*)&p[6], "%u.%u.%u.%u", &b1, &b2, &b3, &b4);
  WiFiConfig.WIFI_TCPIP.IP = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
  system_flags.wifi_on = 1;
  return 0;
}

// послать команду ES8266
#pragma optimize = none
static U16 send_message(pU8 buffer)
{   
  U16 len = sprintf(gsmSendBuf, "%s", buffer);  
  
  USART2_CR1_bit.RE     = 0;
  BytesToTransfer2      = len;
  USART2_CR1_bit.TCIE   = 1;  
  OS_EVENT_Wait( &Uart2TXComplete );
  
  return len;
}

// послать команду ES8266 с проверкой ответа
static U8  SendMessWithAns(U8 *msg, U8 *ans, U32 timeout)
{
  U8 *p;
  
  send_message(msg); 
  if( OS_EVENT_WaitTimed(&Uart2RXComplete, timeout) ) 
  {
    USART2_CR1_bit.RE   = 0;
    return 1; // timeout
  }
  
  p = strstr(gsmReceiveBuf, ans);      
  if( p == NULL )                                     return 2; // no needed ans
                                                      return 0; // success                            
}

// передача данных по WiFi
#pragma optimize = none
static void send_data_WiFi(pU8 buffer, U8 connection, U16 len)
{   
  sprintf( gsmSendBuf, "AT+CIPSEND=%d,%d\r\n", connection, len);
  SendMessWithAns( gsmSendBuf, " ", 100);
  memcpy( (void *)gsmSendBuf, (void *)buffer, len );
  
  USART2_CR1_bit.RE     = 0;
  BytesToTransfer2      = len;
  USART2_CR1_bit.TCIE   = 1;  
  OS_EVENT_Wait( &Uart2TXComplete );
}

// примем данных по WiFi
static U16 recieve_data_WiFi(U32 timeout, pU8 connection, pU8 * datapoint)
{
  pU8   p;
  U32   datanum, connum;
  
  Rx2Counter          = 0;
  USART2_CR1_bit.RE   = 1;
  if( OS_EVENT_WaitTimed(&Uart2RXComplete, timeout) )   
  {
    USART2_CR1_bit.RE   = 0;
    return 1;     // timeout  
  }

  p = strstr( gsmReceiveBuf, "+IPD");  
  if( p == NULL )                                       return 2; // служебный обмен: CONNECT, CLOSED etc
  
  // поиск номепа канала и кол.данных
  p[4] = p[6] = ' ';  
  for(U8 i = 7; i < 13; i++)
  if( p[i] == ':' ) // разделитель данных
  { 
     p[i]       =   ' ';
     *datapoint = &p[i + 1];
     break;
  }
  else if(i == 12) return -1;
  
  sscanf( &p[4], "%u%u", &connum, &datanum);       
  if(datanum > GSM_RECIEVE_LEN)                         return 2;  // too much
    
  *connection = (U8)connum;
  return (U16)datanum;
}
