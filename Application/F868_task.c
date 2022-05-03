#include "defines.h"
#include "sx1276.h"
#include "SX1276Driver.h"

const RadioRegisters_t RadioRegsInit[7] = 
{   
      // 868 MHz
    { REG_LR_FRFMSB           , 0xd9 },
    { REG_LR_FRFMID           , 0x20 },
    { REG_LR_FRFLSB           , 0x04 },
    // PA_BOOST pin out 2 dBm
    { REG_LR_PACONFIG         , RFLR_PACONFIG_PASELECT_PABOOST + 0x70   },
    // LNA Gain-max, 150% LNA current  
    { REG_LR_LNA              , RFLR_LNA_GAIN_G1 + RFLR_LNA_BOOST_HF_ON },
    // OCP - 100 mA
    { REG_LR_OCP              , RFLR_OCP_ON + RFLR_OCP_TRIM_100_MA      },        
    // PAYLOADMAXLENGTH - 64
    { REG_LR_PAYLOADMAXLENGTH , 0x40                                    },        
};

static U8                               spiBuffer[32];

/* OS */
static OS_TIMER                         SX1276_Calib_Timer;

// isr EXTI
#pragma optimize = none
void EXTI15_10_ISR(void)
{
  OS_EnterInterrupt();
  
  switch(EXTI_IMR)
  {
    case (1<<15):     
                  if( SX1276_GetStatus() == RF_RX_RUNNING )       OS_SignalEvent(RECEIVE_RADIO,  &OS_F868);
                  else if( SX1276_GetStatus() == RF_TX_RUNNING )  OS_SignalEvent(TRANSMIT_RADIO, &OS_F868);
                  else                                            OS_SignalEvent(REBAND_RADIO,   &OS_F868);
                  break;                 
  }
  
  EXTI_IMR     = 0;
  EXTI_PR      = 0xffffffff;          // стереть все EXTI события
  
  OS_LeaveInterrupt();
}

// isr SX1276_Calib_Timer
#pragma optimize = none
void SX1276_Calib_ISR( void )
{
  OS_RetriggerTimer ( &SX1276_Calib_Timer );
  OS_SignalEvent    ( REBAND_RADIO, &OS_F868 );
}

void  F868_Task(void)
{
  S16         tempr, rssi;  
  U8          adr, rssi_level, hum, err, bat, wrong_message;
  S8          pres, writePos;
  
  // инициализация радиомодуля  
  errors_flags.radio_fail   = SX1276_Init();
  if(errors_flags.radio_fail) OS_Suspend(NULL);
  
  // прерывания от рмодуля VIC
  OS_ARM_InstallISRHandler(NVIC_EXTI15_10, (OS_ISR_HANDLER*)EXTI15_10_ISR);
  OS_ARM_ISRSetPrio(NVIC_EXTI15_10, 0xFE);
  OS_ARM_EnableISR(NVIC_EXTI15_10);
  
  // прерывания от рмодуля
  AFIO_EXTICR4 = (3<<12);    // PD15  -> EXTI15-10
  EXTI_RTSR    = (1<<15);    // EXTI15-10 rise
  EXTI_FTSR    = 0;
  
  OS_CreateTimer( &SX1276_Calib_Timer, SX1276_Calib_ISR, 300000); 
  OS_STARTTIMER_IF_NOT( &SX1276_Calib_Timer);
  
  // recieve On
  Radio2Rx(MESSAGE_LEN);
    
  for(;;)
  {            
    switch(OS_WaitSingleEvent(0xff))
    {
      default:            // калибровка, смена канала приема, вкл приема, 
reini_lora:        
                          SX1276_Init();
                          Radio2Rx(MESSAGE_LEN);
                          break;
                          
      case RECEIVE_RADIO: // считать данные из буффера приема
                          Radio2IDLE( );                           
                          wrong_message = SX1276_Read1( REG_LR_IRQFLAGS ) & RFLR_IRQFLAGS_PAYLOADCRCERROR;
                          rssi          = SX1276_GetPacketRssi( );
                          rssi_level    = SX1276_GetPacketRssiLevel( );
                          SX1276_ReadFifo( spiBuffer, MESSAGE_LEN);
                          
                          // проверка данных
                          tempr = *((pS16)&spiBuffer[TEMPB1_POS]); // (spiBuffer[TEMPB1_POS] | (((U16)spiBuffer[TEMPB2_POS]) << 8));
                          hum   = spiBuffer[HUM_POS];
                          bat   = spiBuffer[BAT_POS];
                          err   = spiBuffer[ERR_POS];
                          pres  = spiBuffer[PRES_POS];
                          adr   = spiBuffer[ADR_POS];
                                      
                          if     (bat   > 100)                                        wrong_message = 1;
                          else if(tempr > 1000)                                       wrong_message = 1;
                          else if(tempr < -651)                                       wrong_message = 1;
                          else if(hum   > 100)                                        wrong_message = 1;
                          else if( (adr > 128) || (adr == 0) )                        wrong_message = 1;
//                          else if(err & WRONG_ERR)                                    wrong_message = 1;
                          
                          // плохой пакет ?
                          if ( wrong_message ) goto reini_lora;
                                                                                                          
                          OS_Use( &SemaRAM);
                            // найти ячейку для записи
                            writePos = device_seach(adr);
                            // если новый прибор
                            if( writePos < 0 )
                            {
                              if( CurrentCounter >= MAX_CURRENT_POS)
                              { 
                                errors_flags.currentMemEnd = 1; 
                              }
                              else
                              {
                                writePos = ++CurrentCounter - 1;
                                errors_flags.currentMemEnd = 0;
                              }
                            }

                            // есть ячейка для записи
                            if( writePos >= 0 ) 
                            {                            
                              CurrentList[writePos].Time             = Time;
                              CurrentList[writePos].KillTimer        = KillTime;
                              CurrentList[writePos].Tempr            = tempr;
                              CurrentList[writePos].Humidy           = hum;
                              CurrentList[writePos].Pressure         = pres + 700;
                              CurrentList[writePos].PowLev           = bat;
                              CurrentList[writePos].Errors           = err;
                              CurrentList[writePos].Adr              = adr;
                              CurrentList[writePos].RSSI             = rssi_level; 
                              CurrentList[writePos].RECINUSE         = 1;
                              RSSI[writePos]                         = rssi;
                              Add2SaveList(writePos);
                            } 
                          OS_Unuse( &SemaRAM);
                                                                                                        
                          // ретранслировать пакет
                          if( RadioChannelRetrans )
                          {
                            U32  random_wait_time;
                            
                            for(U8 i = 0; i < 3; i++)  // 3 attempts
                            {
                              if( !SX1276_IsChannelFree( (U32)RadioChannelRetrans * CHANEL_SIZE + FREQ_CARRY, LOW_TRESH_FREE ) )  // 
                              {
                                // ждать
                                random_wait_time = ( SX1276_Random( ) + 0.5) * SX1276_TimeOnAir( MESSAGE_LEN ) * 1000;
                                OS_Delay ( random_wait_time );
                                continue;
                              }
                              else
                              {  
                                // ретранслировать
                                Radio2Tx(spiBuffer, MESSAGE_LEN);
                                OS_RetriggerTimer( &SX1276_Calib_Timer);
                                break;
                              }
                            }
                          
                            // если не удалось отправить - вкл прием
                            if( !CHECK_TX )  Radio2Rx(MESSAGE_LEN);
                          }
                          else 
                          // ретранслятор выключен - вкл прием
                          Radio2Rx(MESSAGE_LEN);
                          
                          break;
                          
      case TRANSMIT_RADIO:
                          // пакет передан, вкл прием
                          Radio2Rx(MESSAGE_LEN);
                          break;                           
    }
  }
}
                               
void Radio2IDLE(void)
{
  SX1276_Standby( );
  SWOFF_RX; 
    SWOFF_TX;
}
           
void Radio2Rx(U16 len)
{  
  U8    current_BW_Setting = BW_MODEM + BW_Setting;         // 6...9 - 62...500 kHz

  SX1276_Standby      ( );  
  SX1276_SetChannel   ( (U32)RadioChannelModem * CHANEL_SIZE + FREQ_CARRY );
                      // bandwidth, datarate, coderate, preambleLen, timeout, fixLen, payload, crcOn, freqHopOn, hopPeriod, iqInverted, rxcont
  SX1276_SetRxConfig  ( current_BW_Setting, DATARATE_Setting, CODERATE_Setting, PREAMBLELEN_Setting, 100, true, len, true, false, 0, true, true );  
  SX1276_Rx           ( );
  SX1276_Write1       ( REG_LR_IRQFLAGS, 0xff );
  
  EXTI_PR      = 0xffffffff;          // стереть все EXTI события
  EXTI_IMR     = (1<<15);             // IRQ0 enable  
  LIGHT_RX; 
    SWOFF_TX;
}
                               
void Radio2Tx(pU8 buffer, U8 len)
{
  U8    current_POWEROUT_Setting = (RadioPowerRetrans & 0xf)  + POWEROUT_Setting;   // 2 ... 17 dBm
  U8    current_BW_Setting       = BW_RETRANS                 + BW_Setting;         // 6...9 - 62.5...500 kHz

  SX1276_Standby      ( );  
  SX1276_SetChannel  ( (U32)RadioChannelRetrans * CHANEL_SIZE + FREQ_CARRY );
  SX1276_SetTxConfig ( current_POWEROUT_Setting, current_BW_Setting, DATARATE_Setting, CODERATE_Setting, PREAMBLELEN_Setting, true, true, false, 0, true );  // power, bandwidth, datarate, coderate, preambleLen, fixLen, crcOn, freqHopOn, hopPeriod, iqInverted   
  SX1276_Send        ( buffer, len );
  SX1276_Write1      ( REG_LR_IRQFLAGS, 0xff );
  
  EXTI_PR      = 0xffffffff;          // стереть все EXTI события
  EXTI_IMR     = (1<<15);             // IRQ0 enable      
  LIGHT_TX; 
    SWOFF_RX;
}                
 
// в Save буффер
void Add2SaveList(U8 writePosition)
{
  static U16 locSavedPointer;
      
  // борьба с указателями
  if( SavedCounter >= MAX_SAVED_POS ) 
  {
    if( !errors_flags.savedMemEnd )
    {
      errors_flags.savedMemEnd = 1;
      locSavedPointer = 0;
    }
    if( locSavedPointer >= MAX_SAVED_POS ) locSavedPointer = 0;
  }
  else 
  {
    errors_flags.savedMemEnd = 0;
    locSavedPointer          = SavedCounter;
  }
          
  // внести новую запись
  memcpy((void *) &SavedList[locSavedPointer],(void *) &CurrentList[writePosition], sizeof(radio_list_struct_t));

  // указатель
  if(locSavedPointer != SavedCounter) locSavedPointer++;
  else                                SavedCounter++;
}

S8 device_seach(U8 adr)
{    
  for(U8 i = 0; i < CurrentCounter; i++) 
    if(CurrentList[i].Adr == adr) return i;
  
  return -1;
}

// Save буффер
void ProceedSaveList(void)
{    
  // перемещение прочитанных данных и сдвиг указателя
  U16 p = SavedCounter >> 16;
  if( p )
  {
    SavedCounter &= 0xffff;
    if( p <= SavedCounter)
    {
      memcpy((void *) &SavedList[0], (void *) &SavedList[p], (SavedCounter - p ) * sizeof(radio_list_struct_t) );
      SavedCounter -= p;
    }
  }
}

void  Check4KillTimeOut(void)
{   
  for(U8 i = 0; i < CurrentCounter; i++)
  {
    if( CurrentList[i].KillTimer-- ) continue;
    else
    {
      CurrentCounter--;
      memcpy((void *) &CurrentList[i],(void *) &CurrentList[i + 1], (CurrentCounter - i) * sizeof(radio_list_struct_t) ); // сместить на позицию записи
      memset((void *) &CurrentList[CurrentCounter], 0, sizeof(radio_list_struct_t) );                                     // стереть последнюю запись
    }
  }
}

void sweepCurrentBuff(void) {}
void sweepSaveBuff(void)    {}

/* -------------- */
#pragma optimize=none
U8  shift_byte_spi1(U8 byte)
{
  SPI1_DR = byte; 
  while  (!SPI1_SR_bit.TXE); 
  while  (SPI1_SR_bit.BSY); 
  return SPI1_DR; 
}

/* -------------- */
void Radio_Reset (void)
{
  RES_LOW;
  OS_Delay( 1 );
  RES_HI;
  OS_Delay( 5 );
}

/* -------------- */
void spi_nss (U8 NSS)
{
  if( NSS ) NSS_HI;
  else      NSS_LOW;
}

/* -------------- */
void wait_ms(U32 time)
{
  OS_Delay( time );
}

/* -------------- */
U8 spi_write(U8 byte)
{
  return shift_byte_spi1( byte );
}
