#include "defines.h"
#include "IP_init.h"

// global
OS_TASK                           OS_SUPERVISER;
OS_TASK                           TCP80;
OS_TASK                           TP502;
OS_TASK                           TCP_CMD;
OS_TASK                           OS_UARTS;
OS_TASK                           OS_USB;
OS_TASK                           OS_WIFI;
OS_TASK                           OS_GSM;
OS_TASK                           OS_OLED;
OS_TASK                           OS_F868;
OS_TASK                           OS_MQTT;

OS_STACKPTR U32                   StackOLED[256];
OS_STACKPTR U32                   StackF868[128];
OS_STACKPTR U32                   StackGSMWIFI[128];
OS_STACKPTR U32                   StackUSB[128];
OS_STACKPTR U32                   StackUARTS[128];
//OS_STACKPTR U32                   TCP80Stack[256];
OS_STACKPTR U32                   IP502Stack[256];                // Define the stack of the IP502_Task to 512 bytes
OS_STACKPTR U32                   TCPCMDStack[256];               // Define the stack of the CMD_Task to 512 bytes
OS_STACKPTR U32                   StackSUPERVISER[128];
OS_STACKPTR U32                   StackMQTT[256];

OS_TIMER                          Config_Timer;
OS_TIMER                          KeyTimer;

OS_RSEMA                          SemaRAM;

// static

// predeclaration
void                              Superviser_Task(void);

// main
int main(void) {
  
  OS_IncDI();                      /* Initially disable interrupts  */
  OS_InitKern();                   /* initialize OS                 */
  OS_InitHW();                     /* initialize Hardware for OS    */
  
  // semafores
  OS_CREATERSEMA(&SemaRAM);         // create RAM semafor

  // timers
  OS_CreateTimer( &KeyTimer,     KeyTimerISR, 50);
  OS_CreateTimer( &Config_Timer, Conf_ISR,    5000);
  
  // tasks
  OS_CREATETASK(&OS_SUPERVISER, 0, Superviser_Task, 50, StackSUPERVISER);  
  
  /* Start multitasking */ 
  OS_Start();
  return 0;
}

void  Superviser_Task(void)
{    
  /* проверка памяти конфигурации, загрузка конфигурации */  
  LoadAndCheckConfig();
    
  // communication tasks
  OS_CREATETASK(&OS_UARTS,      0, UARTS_Task,      50, StackUARTS); 
  OS_CREATETASK(&OS_USB,        0, USB_Task,        50, StackUSB); 
  OS_Eth_init(0);                                                                                       /* initialize Hardware for Ethernet */  
  //OS_CREATETASK(&TCP80,         0, TCP80_Task,      50, TCP80Stack);
  OS_CREATETASK(&TP502,         0, IP502_Task,      50, IP502Stack);                                    // Start the port502_Task    
  OS_CREATETASK(&TCP_CMD,       0, UDP_Task,        50, TCPCMDStack);                                   // Start the port1337_Task 
  
  // wireless tasks  
#if     defined(__V2XX__)
  OS_CREATETASK(&OS_F868,       0, F868_Task,       50, StackF868);
  
#elif   defined(__V3XX__)  
  switch(ProtocolNumber)                                                                                // Start the F868_Task 
  {
    case 0:       OS_CREATETASK(&OS_F868,       0, F868_Task_1,       50, StackF868);
                  break;
                  
    case 1:       OS_CREATETASK(&OS_F868,       0, F868_Task_2,       50, StackF868); 
                  break;                 
                  
    default: ;                  
  }  
#endif
  
  OS_EVENT_Create ( &Uart2RXComplete );
  OS_EVENT_Create ( &Uart2TXComplete );
  
  if    (ConfigFlags & GSM_WIFI ) { WiFiConfig.wifi_present = 0; GSMConfig.gsm_present = 1; OS_CREATETASK(&OS_GSM,  0,  GSM_Task,  50, StackGSMWIFI); }   
  else                            { WiFiConfig.wifi_present = 1; GSMConfig.gsm_present = 0; OS_CREATETASK(&OS_WIFI, 0, WIFI_Task,  50, StackGSMWIFI); }   
        
  // config
  OLED_ini(); 
  GUI_LoadingScreen("ТЕСТ КОНФИГУРАЦИИ...", !errors_flags.config_fail, "ЗАГРУЗИТЕ КОНФИГУРАЦИЮ", 1, 4); 
  if(errors_flags.config_fail)
  {
    while(1) // stop!
    {
      CLEAR_IWDG;
      OS_Delay(500); 
    }        
  }  
  OS_Delay(1500);
  CLEAR_IWDG;
    
  // rtc   
  GUI_LoadingScreen("ТЕСТ RTC...", !errors_flags.timeinvalid, " УСТАНОВИТЕ ВРЕМЯ ", 2, 4); 
  OS_Delay(1500); 
  CLEAR_IWDG;  
      
  // radio433
  GUI_LoadingScreen("ТЕСТ РАДИО МОДУЛЯ...", !errors_flags.radio_fail, "МОДУЛЬ НЕИСПРАВЕН!", 3, 4); 
  if(errors_flags.radio_fail) 
  {
    while(1) // stop!
    {
      CLEAR_IWDG;
      OS_Delay(500);
    }    
  }
  OS_Delay(1500);
  CLEAR_IWDG;  
  
  // wifi - gsm
  if(WiFiConfig.wifi_present) 
  {
    GUI_LoadingScreen("ТЕСТ WiFi МОДУЛЯ...", !errors_flags.wifi_fail, "МОДУЛЬ НЕИСПРАВЕН!", 4, 4);    
  }
  else if (GSMConfig.gsm_present)
  {
    GUI_LoadingScreen("ТЕСТ GSM МОДУЛЯ...", !errors_flags.gsm_fail, "МОДУЛЬ НЕИСПРАВЕН!", 4, 4); 
  }
  OS_Delay(1500);
  CLEAR_IWDG;  
    
  // Старт экрана и клавиатуры, mqtt 
  OS_CREATETASK         ( &OS_OLED, 0, OLED_Task, 50, StackOLED);
  OS_CREATETASK         ( &OS_MQTT, 0, MQTT_Task, 50, StackMQTT);
  OS_STARTTIMER_IF_NOT  ( &KeyTimer);
  OS_RESUME_IF_NOT      ( &OS_F868 );  
  
  /* задача выполняется с периодом 1000 мс */
  U8  usbBugClock            = 0;
  system_flags.hotreboot_ini = 1;  
  
  for(U32 superviserWaitTime = 1000, hotResTime = 0; ; CLEAR_IWDG)
  {
    // сброс WDT
    OS_Delay(superviserWaitTime);
    U32 startLoopTime = OS_GetTime32();
    
    OS_Use( &SemaRAM);
    
      // время для внешнего обзора
      Time = RTC_GetCounter(); 
      
      // проверка
      Check4KillTimeOut();
      if( ConfigFlags & CURR_BUFF_SWEEP) sweepCurrentBuff();
      if( ConfigFlags & SAVE_BUFF_SWEEP) sweepSaveBuff();
      
      // проверка времени последней синхронизации
      U32 timeSync = get_bkp_reg(TIMESYNC_BASE) + ((U32)get_bkp_reg(TIMESYNC_BASE + 1) << 16);    
      if( (timeSync + NTPConfig.periodValidTime)  < Time ) errors_flags.TimeSyncReq  = 1;  
      if( (timeSync + NTPConfig.periodUpdateTime) < Time ) errors_flags.NTPSyncReq   = 1;  
      
      // горячий перезапуск
      if( system_flags.hotreboot_ini )              
      {
        system_flags.hotreboot_ini = 0;
        hotResTime                 = RTC_GetCounter() + HotRebootTime;
      }
      if( ( hotResTime < Time ) && HotRebootTime )  system_flags.start_boot = 1;
      
    OS_Unuse( &SemaRAM);
        
    // активные интерфейсы
    system_flags.ethernet_on = IP_IFaceIsReady(0);
    system_flags.usb_on      = USB_IsConfigured();
    
    // сброс устройства
    if(errors_flags.radio_fail || system_flags.start_boot)
    {
      if( IS_USB_CONNECT )
      {
        USB_DeInit();
        OS_Delay(200);
      }
      HW_RESET;
    }
    
    // подъем USB - 60 с таймаут
    if( IS_USB_CONNECT && !system_flags.usb_on )  
    {
      usbBugClock++;
      if( usbBugClock == 60 ) 
      {
        GPIOA_CRH &= 0xffffff0f;
        GPIOA_CRH |= 0x00000020;
      }
    }
    else                                          
    {
      usbBugClock = 0;
      if( (GPIOA_CRH & 0x000000f0) == 0x00000020 ) 
      {
        GPIOA_CRH &= 0xffffff0f;
        GPIOA_CRH |= 0x00000040;
      }
    }
        
    // расчет времени следующего цикла
    startLoopTime = OS_GetTime32() - startLoopTime; 
    if(startLoopTime < 1000)  superviserWaitTime  = 1000 - startLoopTime; 
    else                      superviserWaitTime  = 0;
  }
}
