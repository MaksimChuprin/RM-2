#include "defines.h"

// global
static U8                         Uart1Buffer[U1BUFLEN];
static U16                        Rx1Counter;
static U16                        BytesToTransfer1;


extern OS_TIMER                   USB_Config_Timer;
// local static

// установка скорости UARTов
uart_speed_stuct select_speed(U32 speed, U8 apbn)
{
 U16                              br = 0x1d4c;
 F32                              time_out;
 uart_speed_stuct                 res;
  
 switch(apbn)
 {          
  case APB1:            
            switch(speed)
            {
            case  1200:   time_out  = 0.06;
                          br        = 0x7530;
                          break;
                          
            case  2400:   time_out  = 0.045;
                          br        = 0x3A98;
                          break;
                          
            case  4800:   time_out  = 0.03;
                          br        = 0x1d4c;
                          break;                          
                          
            case  9600:   time_out  = 0.02;
                          br        = 0xea6;
                          break;
                          
            case  19200:  time_out  = 0.02;
                          br        = 0x753;
                          break;
                          
            case  38400:  time_out  = 0.01;
                          br        = 0x3a9;
                          break;
                          
            case  57600:  time_out  = 0.005;
                          br        = 0x271;
                          break;
                          
            case  115200: time_out  = 0.005;
                          br        = 0x139;
                          break;
                          
            default:      time_out  = 0.02;
                          br        = 0xea6;
                          break;
            }
            break;
          
  case APB2: switch(speed)
            {
            case  1200:   br        = 0xEA60;
                          time_out  = 0.06;
                          break;
                          
            case  2400:   br        = 0x7530;
                          time_out  = 0.045;
                          break;
                          
            case  4800:   br        = 0x3A98;
                          time_out  = 0.03;
                          break;                          
                          
            case  9600:   br        = 0x1d4c;
                          time_out  = 0.02;
                          break;
                          
            case  19200:  br        = 0xea6;
                          time_out  = 0.02;
                          break;
                          
            case  38400:  br        = 0x753;
                          time_out  = 0.01;
                          break;
                          
            case  57600:  br        = 0x4e2;
                          time_out  = 0.005;
                          break;
                          
            case  115200: br        = 0x271;
                          time_out  = 0.005;
                          break;
                          
            default:      br        = 0x1d4c;
                          time_out  = 0.02;
                          break;                                        
            }
            break;
  }
 
  res.speed = br;
  res.time  = time_out;
  return res;
}


// isr TIM3
#pragma optimize = none
void  TIM3_ISR(void)
{
  OS_EnterInterrupt();
  
  TIM3_SR           &=  ~BIT1;
  TIM3_CR1          &=  ~BIT0;
  USART1_CR1_bit.RE  =   0;
  OS_SignalEvent( BIT0, &OS_UARTS);
  
  OS_LeaveInterrupt();
}

// isr uart1
#pragma optimize = none
void UART1_ISR(void) 
{
  static U16    txCounter;
  U8            inByte;
  U32           uart_status;
  
  OS_EnterInterrupt();
  
  uart_status  = USART1_SR;
  inByte       = USART1_DR;  
  
  if(uart_status & (0x0f)) // FE NE ORE - errors
  {
    Rx1Counter    =      0;
    TIM3_CR1     &=  ~BIT0;
    TIM3_SR      &=  ~BIT1;
  }
  else
  if(uart_status & (1<<5))  // RXNE
  {        
    if(Rx1Counter == U1BUFLEN)   
    { 
      Rx1Counter    = 0; 
      TIM3_CR1     &=  ~BIT0;
      TIM3_SR      &=  ~BIT1;
    }
    else  
    {
      Uart1Buffer[Rx1Counter++] = inByte;     
      //запустить или обнулить таймер
      TIM3_EGR      |=   BIT0;
      TIM3_CR1      |=   BIT0;
    }
  }
  
  if(USART1_CR1_bit.TCIE && (uart_status & (1<<6))) // TC
  {    
    if((txCounter == BytesToTransfer1) || (txCounter == U1BUFLEN))
    {
      txCounter =           0;
      U1RS485_OFF;
      USART1_CR1_bit.TCIE=  0;
      USART1_CR1_bit.RE=    1;
      if(system_flags.reiniuart1) 
      {
        uart_speed_stuct  speedset = select_speed( RsSpeed, APB2);
        U32               tim3     = speedset.time * 36000000;
        
        USART1_BRR =    speedset.speed;
        TIM3_ARR   =    TIM3_CCR1 = (U16)tim3; 
        TIM3_PSC   =    (U16)(tim3 >> 16);
      }
    } 
    else USART1_DR = Uart1Buffer[txCounter++];
  }
  
  OS_LeaveInterruptNoSwitch();
}

void changeRSspeed(void)
{
  U32               tim3;
  uart_speed_stuct  speedset;
  
  USART1_CR1_bit.UE =  0;
  speedset          =  select_speed( RsSpeed, APB2);
  USART1_BRR        =  speedset.speed;
  tim3              =  speedset.time * 36000000;
  TIM3_ARR          =  TIM3_CCR1 = (U16)tim3; 
  TIM3_PSC          =  (U16)(tim3 >> 16);
  USART1_CR1_bit.UE =  1;
}   

/* #################  TASK ##########################  */
void UARTS_Task(void) 
{
  U16               rxnum;
  
  // инициализация UART 
  USART1_CR1= USART1_CR2 = USART1_CR3= USART1_GTPR= 0;
  USART1_CR1_bit.UE =      0;
  USART1_CR1_bit.RE =      1;
  USART1_CR1_bit.TE =      1;
  USART1_CR1_bit.RXNEIE =  1;
  
  // инициализация TIM3
  TIM3_CR1   = 0;                   // 
  TIM3_CCMR1 = BIT4;                // CC1 
  TIM3_DIER  = BIT1;                // CC1IE
  
  // start UART  
  changeRSspeed();
  
  // инициализация в VIC   
  OS_ARM_InstallISRHandler(NVIC_USART1, (OS_ISR_HANDLER*)UART1_ISR);
  OS_ARM_ISRSetPrio(NVIC_USART1, 0xFE);
  OS_ARM_EnableISR(NVIC_USART1); 
  
  OS_ARM_InstallISRHandler(NVIC_TIM3, (OS_ISR_HANDLER*)TIM3_ISR);
  OS_ARM_ISRSetPrio(NVIC_TIM3, 0xFE);
  OS_ARM_EnableISR(NVIC_TIM3);
    
  for(;;)
  {
    OS_WaitSingleEvent(BIT0);
    
    if(!Rx1Counter)
    {
      USART1_CR1_bit.RE = 1;
      continue;
    }
    
    rxnum       = Rx1Counter;
    Rx1Counter  = 0;      
   
    if ((Uart1Buffer[0] == '$') && (Uart1Buffer[rxnum - 1] == 0x0d) && (check_sum8(Uart1Buffer, rxnum - 3) == hex_to_char(rxnum - 3, Uart1Buffer)))
    // if EKSIS
    {
      BytesToTransfer1 = EKSIS_ASCII(Uart1Buffer);
      if(!BytesToTransfer1)
      {
        USART1_CR1_bit.RE = 1;
      }
      else
      {
        U1RS485_ON;
        USART1_CR1_bit.TCIE = 1; 
      }
    }
    // if MODBUS
    else
    {
      BytesToTransfer1 = MODBUS_RTU(Uart1Buffer, RsAdr);
      if(!BytesToTransfer1)  
      {
        USART1_CR1_bit.RE = 1;
      }
      else
      {
        U1RS485_ON;
        USART1_CR1_bit.TCIE = 1; 
      }
    }
  }
}

// ....
U32 EKSIS_ASCII(pU8 buffer)
{
  U32     addr, command, i, bytesToTransfer;
  U8      chksum;
  
  addr = hex_to_int(1, buffer);
  if( (addr != RsAdr) && (addr != 0xffff)  )  return 1;
  
  addr            = hex_to_int(7, buffer);
  bytesToTransfer = hex_to_char(11, buffer);
  command         = buffer[5]*256 + buffer[6];
  buffer[0]       = '!';
    
  switch(command)
  {
    
     case 'WI':   if((bytesToTransfer > MAX_DATA_LEN_UART1) || ((addr + bytesToTransfer) >  CONFIGSIZE))   goto fail;
                  OS_RetriggerTimer( &Config_Timer ); 
                  OS_Use( &SemaRAM); 
                    hexbuffer_2_binbuffer( buffer, (pInt8U)(ConfigBeginAddr + addr), bytesToTransfer, 13); 
                  OS_Unuse( &SemaRAM); 
                  bytesToTransfer = 7; 
                  goto final;
                  
     case 'WR':   if( (bytesToTransfer > MAX_DATA_LEN_UART1) || ((addr + bytesToTransfer) > RAMSIZE) ) goto fail;
                  OS_Use( &SemaRAM);
                    hexbuffer_2_binbuffer(buffer, (pInt8U)(RamBeginAddr + addr), bytesToTransfer, 13);
                    if( (RamBeginAddr + addr) == (U32)&Time )  // RTC
                    {
                      RTC_SetCounter( Time );
                      
                      errors_flags.TimeSyncReq = errors_flags.timeinvalid = 0; 
                      set_bkp_reg( ERR_BASE, 0);
                      set_bkp_reg( TIMESYNC_BASE,     Time & 0xffff);
                      set_bkp_reg( TIMESYNC_BASE + 1, Time >> 16);
                    }
                    ProceedSaveList();
                  OS_Unuse( &SemaRAM);
                  bytesToTransfer= 7;
                  goto final;

     case 'RR':   if( (bytesToTransfer > MAX_DATA_LEN_UART1) || ((addr + bytesToTransfer) >  RAMSIZE) ) goto fail;
                  OS_Use( &SemaRAM);
                    binbuffer_2_hexbuffer(buffer,(pInt8U)(RamBeginAddr + addr), bytesToTransfer, 7);
                  OS_Unuse( &SemaRAM);
                  bytesToTransfer=  7 + bytesToTransfer * 2;
                  goto final;
                  
     case 'RI':   if((bytesToTransfer > MAX_DATA_LEN_UART1) || ((addr + bytesToTransfer) >  CONFIGSIZE)) goto fail;
                  binbuffer_2_hexbuffer(buffer, (pU8)(ConfigFlashBeginAddr + addr), bytesToTransfer, 7); 
                  bytesToTransfer=  7 + bytesToTransfer*2;
                  goto final;
                  
     case 'RF':   if(bytesToTransfer > MAX_DATA_LEN_UART1) goto fail;
                  binbuffer_2_hexbuffer(buffer, (pU8)((U32)main_screen + addr), bytesToTransfer, 7);
                  bytesToTransfer = bytesToTransfer * 2 + 7;
                  goto final;

     case 'IR':   bytesToTransfer= 7;
                  for(i = 0; i<8; i++, bytesToTransfer++)        buffer[bytesToTransfer]= SerialNumber[i];
                  buffer[bytesToTransfer++]= ' ';
                  for(i = 0; Version[i]; i++, bytesToTransfer++) buffer[bytesToTransfer]= Version[i];
                  buffer[bytesToTransfer++]= ' ';
                  for(i = 0; ID[i]; i++, bytesToTransfer++)      buffer[bytesToTransfer]= ID[i];
                  buffer[bytesToTransfer++]= ' ';
                  goto final;
                                                      
    default:
fail:             buffer[0]       =   '?';
                  bytesToTransfer =   7;
final:
                  chksum = check_sum8(buffer, bytesToTransfer);
                  binbuffer_2_hexbuffer(buffer, &chksum, 1, bytesToTransfer);
                  bytesToTransfer               += 2;
                  buffer[bytesToTransfer++]      = 0x0d;                                        
  }
  return bytesToTransfer;
}
