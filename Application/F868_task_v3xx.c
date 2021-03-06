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

/* OS */
static OS_TIMER               SX1276_Calib_Timer;

#pragma data_alignment = 4
static  U8                    spiBuffer[MAXLORAMESSAGELEN];
#pragma data_alignment = 4
static  radio_list_struct_t   pre_measure;

// isr EXTI
#pragma optimize = none
void EXTI15_10_ISR(void)
{
  OS_EnterInterrupt();
  
  if( SX1276_GetStatus() == RF_RX_RUNNING )       OS_SignalEvent(RECEIVE_RADIO,  &OS_F868);
  else if( SX1276_GetStatus() == RF_TX_RUNNING )  OS_SignalEvent(TRANSMIT_RADIO, &OS_F868);
  else                                            OS_SignalEvent(REBAND_RADIO,   &OS_F868);
  
  EXTI_IMR     = 0;
  EXTI_PR      = 0xffffffff;          // ??????? ??? EXTI ???????
  
  OS_LeaveInterrupt();
}

// isr SX1276_Calib_Timer
#pragma optimize = none
void SX1276_Calib_ISR( void )
{
  OS_RetriggerTimer ( &SX1276_Calib_Timer    );
  OS_SignalEvent    ( REBAND_RADIO, &OS_F868 );
}

// ???????????????? ????????
void  F868_Task_1(void)
{  
  // ????????????? ???????????  
  errors_flags.radio_fail   = SX1276_Init();
  if(errors_flags.radio_fail) OS_Suspend(NULL);
  
  // ?????????? ?? ??????? VIC
  OS_ARM_InstallISRHandler(NVIC_EXTI15_10, (OS_ISR_HANDLER*)EXTI15_10_ISR);
  OS_ARM_ISRSetPrio(NVIC_EXTI15_10, 0xFE);
  OS_ARM_EnableISR(NVIC_EXTI15_10);
  
  // ?????????? ?? ???????
  AFIO_EXTICR4 = (3<<12);    // PD15  -> EXTI15-10
  EXTI_RTSR    = (1<<15);    // EXTI15-10 rise
  EXTI_FTSR    = 0;
  
  OS_CreateTimer( &SX1276_Calib_Timer, SX1276_Calib_ISR, 300000); 
  OS_STARTTIMER_IF_NOT( &SX1276_Calib_Timer);
  
  // recieve On
  Radio2Rx(RX_DATA, ProtocolNumber);
  
  U8          irqFlags;
  S8          writePos;  
    
  for(;;)
  {            
    switch(OS_WaitSingleEvent(0xff))
    {
      default:            // ??????????, ????? ?????? ??????, ??? ??????
                          errors_flags.radio_fail = SX1276_Init();
                          Radio2Rx(RX_DATA, ProtocolNumber);
                          break;
                          
      case RECEIVE_RADIO: // ??????? ?????? ?? ??????? ??????
                          Radio2IDLE( );                           
                          irqFlags = SX1276_Read1( REG_LR_IRQFLAGS );
                          if( irqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR )  { Radio2Rx(RX_DATA, ProtocolNumber); break; }
                          
                          SX1276_ReadFifo( spiBuffer, 7);
                          
                          pre_measure.RSSI     =  SX1276_GetPacketRssi( );
                          pre_measure.Adr      =  spiBuffer[0];
                          pre_measure.Tempr    = *((pS16)&spiBuffer[1]); 
                          pre_measure.Humidy   =  spiBuffer[3];                           
                          pre_measure.PowLev   =  spiBuffer[4];
                          pre_measure.Errors   =  spiBuffer[5];
                          pre_measure.Pressure =  700 + (S8)spiBuffer[6];
                          
                          pre_measure.Type     =  IVTM_7M4_OKW;                        
                          pre_measure.Config   =  0;           
                          pre_measure.Time     =  Time;  
                          
                          if( pre_measure.Humidy > 100 )      { Radio2Rx(RX_DATA, ProtocolNumber); break; };
                          if( pre_measure.Pressure > 828 )    { Radio2Rx(RX_DATA, ProtocolNumber); break; };
                          if( pre_measure.Pressure < 573 )    { Radio2Rx(RX_DATA, ProtocolNumber); break; };
                          if( pre_measure.Tempr > 1550 )      { Radio2Rx(RX_DATA, ProtocolNumber); break; };
                          if( pre_measure.Tempr < -650 )      { Radio2Rx(RX_DATA, ProtocolNumber); break; };  
                          if( pre_measure.Adr == 0 )          { Radio2Rx(RX_DATA, ProtocolNumber); break; };
                          if( pre_measure.PowLev > 100 )      { Radio2Rx(RX_DATA, ProtocolNumber); break; };                                                    
                                                                                                                                    
                          // ????? ?????? ??? ??????
                          writePos = device_seach(pre_measure.Adr);
                          
                          if( writePos < 0 )
                          {
                            // ????? ??????
                            if( CurrentCounter >= MAX_CURRENT_POS)
                            { 
                              errors_flags.currentMemEnd = 1;
                              Radio2Rx(RX_DATA, ProtocolNumber);                                                  // ????????? ?????? - ?? ?????, ?? ?????????????
                              break;
                            }
                            else
                            {
                              errors_flags.currentMemEnd = 0; 
                              writePos                   = CurrentCounter;      
                              OS_Use( &SemaRAM);                               
                                ++CurrentCounter;
                              OS_Unuse( &SemaRAM);
                            }
                          }
                          
                          OS_Use( &SemaRAM);
                            memcpy( &CurrentList[writePos], &pre_measure, sizeof(radio_list_struct_t) );      // ????????? ? CurrentList
                          OS_Unuse( &SemaRAM);
                          
                          // ????????? ? SavedList
                          Add2SaveList();
                                                                                                        
                          // ??????????????? ?????
                          if( RadioChannelRetrans )
                          {
                            Radio2Tx(spiBuffer, 7, TX_DATA, ProtocolNumber);
                            OS_RetriggerTimer( &SX1276_Calib_Timer);                          
                          }
                          else Radio2Rx(RX_DATA, ProtocolNumber);                                                 // ???????????? ???????? - ??? ?????
                                                                              
                          break;
                          
      case TRANSMIT_RADIO:
                          // ????? ???????, ??? ?????
                          Radio2Rx(RX_DATA, ProtocolNumber);
                          break;                           
    }
  }
}

// ??????????????? ????????
void  F868_Task_2(void)
{    
  // ????????????? ???????????  
  errors_flags.radio_fail   = SX1276_Init();
  if(errors_flags.radio_fail) OS_Suspend(NULL);
  
  // ?????????? ?? ??????? VIC
  OS_ARM_InstallISRHandler(NVIC_EXTI15_10, (OS_ISR_HANDLER*)EXTI15_10_ISR);
  OS_ARM_ISRSetPrio(NVIC_EXTI15_10, 0xFE);
  OS_ARM_EnableISR(NVIC_EXTI15_10);
  
  // ?????????? ?? ???????
  AFIO_EXTICR4 = (3<<12);    // PD15  -> EXTI15-10
  EXTI_RTSR    = (1<<15);    // EXTI15-10 rise
  EXTI_FTSR    = 0;

  // ?????? ??????????? ??????????  
  OS_CreateTimer( &SX1276_Calib_Timer, SX1276_Calib_ISR, 300000);
  
  U32   timeout_ans;
  U32   external_time;
  U8    wrongMessageFlag;

  U8    messageLen;
  U8    eventSX1276;
  S8    writePos; 
  S8    badPacketCount = 0;
  U8    irqFlags = 0;
    
  U8    mod_mode    = MODEM_MODE;
  U32   modemTime   = OS_GetTime32() + ModemMaxTime * 1000;
  U32   deviceTime  = 0;
  U16   crc = 0;
     
  OS_Suspend(NULL);
  OS_SignalEvent ( REBAND_RADIO, &OS_F868 );
  OS_STARTTIMER_IF_NOT( &SX1276_Calib_Timer);
      
  for(;;)
  {
    // ???????? ???????  ?????-??????????
    switch( mod_mode )
    {
      case MODEM_MODE:  if( (modemTime < OS_GetTime32()) && (SX1276_GetStatus() == RF_RX_RUNNING) )   // ? ?????? ? ????? ?????
                        {
                          if ( SavedCounter && RadioChannelRetrans )                                  // ???? ??? ?????????? ? ??? ????? ?????????????
                          {
                            errors_flags.radio_fail = SX1276_Init();
                            DISABLE_LORA_INT;
                            mod_mode     = DEVICE_MODE;
                            OS_ClearEvents(&OS_F868);
                            deviceTime   = OS_GetTime32() + ReTransMaxTime * 1000;
                            timeout_ans  = 1;
                          }
                          else modemTime = OS_GetTime32() + ModemMaxTime * 1000;  // ??? ??????, ????????? ???????
                        }
                        break;
                        
      case DEVICE_MODE: if( (deviceTime < OS_GetTime32() )  && (SX1276_GetStatus() == RF_TX_RUNNING) ) // ????? ?????
                        {
                          errors_flags.radio_fail = SX1276_Init();
                          DISABLE_LORA_INT;
                          mod_mode     = MODEM_MODE;
                          OS_ClearEvents(&OS_F868);
                          modemTime    = OS_GetTime32() + ModemMaxTime * 1000;
                          Radio2Rx(RX_DATA, ProtocolNumber);
                        }
                        break;
    }
                                
    switch( mod_mode )
    {
/* ####################################  ????? ????? - ?????  ############################################ */        
      case MODEM_MODE:  eventSX1276 = OS_WaitSingleEventTimed(0xff, 1000);
      
                        switch(eventSX1276)
                        {                                                        
                          case REBAND_RADIO:          // ??????????, ????? ?????? ??????, ??? ??????
                                    Radio2IDLE();
                                    DISABLE_LORA_INT; OS_ClearEvents(&OS_F868);
                                    errors_flags.radio_fail = SX1276_Init();
                                    Radio2Rx(RX_DATA, ProtocolNumber);
                                    break;
                          
                          case RECEIVE_RADIO: // ??????? ?????? ?? ??????? ??????
                                    Radio2IDLE( ); 
//                                    RadioDebug.Total_Received_Count++;
                                    irqFlags   = SX1276_Read1( REG_LR_IRQFLAGS );
                                    messageLen = SX1276_Read1( REG_LR_RXNBBYTES );
                                    
                                    // ???????? ?????? CRC, Meslen, Header
                                    if( (irqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR ) || !(irqFlags & RFLR_IRQFLAGS_RXDONE ) || !(irqFlags & RFLR_IRQFLAGS_VALIDHEADER ) ||
                                        (messageLen > MAXLORAMESSAGELEN) )  
                                    { 
//                                      RadioDebug.Packet_Error_Count++;
                                      badPacketCount++;
                                      if( badPacketCount > 10 )
                                      {
                                        errors_flags.radio_fail = SX1276_Init();
                                        badPacketCount = 0;
                                      } 
                                      Radio2Rx(RX_DATA, ProtocolNumber); 
                                      break; 
                                    }
                                    
                                    // ?????? ??????
                                    SX1276_ReadFifo( spiBuffer, messageLen );

                                    // ???????? ?? ????? ??????
                                    if( spiBuffer[CHANEL_POS] != 0 )
                                    {
                                      if( spiBuffer[CHANEL_POS] != RadioChannelModem ) 
                                      {
//                                        RadioDebug.Data_Error_Count++;
                                        badPacketCount++;
                                        if( badPacketCount > 10 )
                                        {                                          
                                          errors_flags.radio_fail = SX1276_Init();
                                          badPacketCount = 0;
                                        } 
                                        Radio2Rx(RX_DATA, ProtocolNumber); 
                                        break;
                                      }
                                    }
                                                                                                                  
                                    pre_measure.RSSI    =  SX1276_GetPacketRssi( );
                                    pre_measure.Type    =  spiBuffer[TYPE_POS];
                                    pre_measure.Adr     =  spiBuffer[ADR_POS];
                                    pre_measure.Time    = *((pU32)&spiBuffer[TIME_POS]);
                                    pre_measure.Config  = *((pU16)&spiBuffer[CONFIG_POS]);
                                    pre_measure.Errors  = *((pU16)&spiBuffer[ERRORS_POS]);
                                    pre_measure.PowLev  =  spiBuffer[POWLEV_POS]; 
                                                                        
                                    // time bug!
                                    if( pre_measure.Time > Time ) pre_measure.Time = Time;
                          
                                    switch( pre_measure.Type )
                                    {
                                      case IVTM_7M4_OKW:
                                      case IVTM_7M4_PRT:  wrongMessageFlag = Parse_7M4( messageLen );
                                                          break;
                                                
                                      case MAG_7:         wrongMessageFlag = Parse_MAG7( messageLen ); 
                                                          break;
                                                
                                      default:            wrongMessageFlag = 1;
                                    }                          
                                    if ( wrongMessageFlag ) 
                                    { 
//                                      RadioDebug.Data_Error_Count++;
                                      Radio2Rx(RX_DATA, ProtocolNumber); 
                                      break; 
                                    }
                                                                              
                                    // ????? ?????? ??? ??????
                                    writePos = device_seach( pre_measure.Adr );
                                                    
                                    if( writePos < 0 )
                                    {   
                                      //  ????? ??????
                                      if( CurrentCounter >= MAX_CURRENT_POS)
                                      { 
                                        errors_flags.currentMemEnd = 1; 
                                        SendResponse(NOASKTIME_LORA);                                                     // ????????? ?????? - ?? ????????????
                                      }
                                      else
                                      {
                                        errors_flags.currentMemEnd = 0; 
                                        OS_Use( &SemaRAM);
                                          writePos = CurrentCounter;                                        
                                          memcpy( &CurrentList[writePos], &pre_measure, sizeof(radio_list_struct_t) );    // ????????? ? CurrentList
                                          ++CurrentCounter;
                                        OS_Unuse( &SemaRAM);
                                        
                                        if( Add2SaveList( ) == NOERR )          SendResponse(ASKTIME_LORA);               // ????????? ? SavedList ? ??????????? 
                                        else                                    SendResponse(NOASKTIME_LORA);             // ????????? ?????? - ?? ????????????
                                      }
                                    }
                                    else 
                                    {
                                      //  ????????? ??????
                                      if( CurrentList[writePos].Time < pre_measure.Time ) 
                                      {
                                        OS_Use( &SemaRAM);
                                          writePos = device_seach( pre_measure.Adr );  // ????????? ????? ?? ?????? sweepCurrentBuff() ? Check4KillTimeOut()
                                          if( writePos >= 0 ) memcpy( &CurrentList[writePos], &pre_measure, sizeof(radio_list_struct_t) );  // ????????? ? CurrentList  
                                        OS_Unuse( &SemaRAM);
                                      }
                                      
                                      if( CheckSaveListDupl( ) )                SendResponse(ASKTIME_LORA);             // ??????????? ????? (SavedList)- ?? ?????????, ?? ????????????
                                      else if( Add2SaveList( ) == NOERR )       SendResponse(ASKTIME_LORA);             // ????????? ? SavedList ? ???????????
                                      else                                      SendResponse(NOASKTIME_LORA); // Radio2Rx(RX_DATA, ProtocolNumber);      // ????????? ?????? - ?? ????????????
                                    }
                                    break;
                          
                          case TRANSMIT_RADIO:  // ????? ???????, ??? ?????
                                    Radio2Rx(RX_DATA, ProtocolNumber);
                                    break;                                                                       
                        }
                        break;
/* ####################################  ????? ????? - ?????  ############################################ */ 
    
/* ####################################  ????? ?????????? - ???????????? ################################# */     
      case DEVICE_MODE: eventSX1276 = OS_WaitSingleEventTimed(0xff, timeout_ans);
      
                        switch(eventSX1276)
                        {
                            case REBAND_RADIO:                        // ?????? ????? ?????? ????????????? 
                                    deviceTime = 0;
                                    Radio2Tx(spiBuffer, 16, TX_DATA, ProtocolNumber); // ????????? ? ????? Tx ??? ?????? ? ????? ??????
                                    break;
                                                
                            case TRANSMIT_RADIO:
                                    timeout_ans = SX1276_TimeOnAir( 16 ) * 1000 + 200;  // 16 ???? - ???????????? ?????
                                    Radio2Rx(RX_ANS, ProtocolNumber);
                                    break;                                                                           
                          
                            case RECEIVE_RADIO: // ??????? ?????? ?? ??????? ??????
                                    Radio2IDLE(); 
                                    irqFlags   = SX1276_Read1( REG_LR_IRQFLAGS );
                                    messageLen = SX1276_Read1( REG_LR_RXNBBYTES );
                                    
                                    if( (irqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR ) || !(irqFlags & RFLR_IRQFLAGS_VALIDHEADER) || (messageLen > MAXLORAMESSAGELEN) )
                                    { 
retr_err:                                      
                                      // ?????? ????? - ?????? ????????
                                      Radio2Tx(spiBuffer, messageLen, TX_DATA, ProtocolNumber);
                                      timeout_ans = SX1276_TimeOnAir( messageLen + 5 ) * 1000;
                                      break;
                                    }
                                                    
                                    SX1276_ReadFifo( spiBuffer, messageLen );
                                    // ???????? ??????
                                    if( spiBuffer[ANS_ADR_POS] != SavedList[SavedCounter - 1].Adr ) goto retr_err;
                                    
                                    // ?????? ??????
                                    switch( spiBuffer[ANS_CMD_POS] )
                                    {                
                                    case ASKTIME_LORA:    switch( messageLen )
                                                          {
                                                            case ASKTIME_LEN:     break;
                                                            case ASKTIME_LEN_CRC: crc = *((pU16)&spiBuffer[ANS_CRC_POS]); 
                                                                                  if( crc != GetModbusRTUCRC(spiBuffer, ASKTIME_LEN_CRC - 2) ) goto retr_err;
                                                                                  break;
                                                            
                                                          default: goto retr_err;
                                                          }
                                    
                                                          external_time = spiBuffer[2] + ((U32)spiBuffer[3] << 8) + ((U32)spiBuffer[4] << 16) + ((U32)spiBuffer[5] << 24);                                                           
                                                          
                                                          if( errors_flags.TimeSyncReq || errors_flags.timeinvalid )
                                                          {
                                                            OS_Use( &SemaRAM);
                                                              Time = external_time; 
                                                              RTC_SetCounter( Time );                                                             
                                                            OS_Unuse( &SemaRAM);
                                                          }
                                                          
                                                          errors_flags.TimeSyncReq = errors_flags.timeinvalid = 0; 
                                                          set_bkp_reg( ERR_BASE, 0);
                                                          set_bkp_reg( TIMESYNC_BASE,     Time & 0xffff);
                                                          set_bkp_reg( TIMESYNC_BASE + 1, Time >> 16);
                                                          if( SavedCounter ) SavedCounter--;  // ?????? ???????? - ?????? ?? SavedList
                                                          break;
                                                          
                                      case ASK_LORA:      if( messageLen != ASK_LEN ) goto retr_err;
                                                          if( SavedCounter ) SavedCounter--;  // ?????? ???????? - ?????? ?? SavedList
                                                          break;
                                                          
                                      case NOASK_LORA:
                                      case NOASKTIME_LORA:
                                                          deviceTime = 0; // ?????? ? ????? ??????
                                                          break;
                                        
                                      default:            goto retr_err;
                                    }
                                    
                                    // ????????? ?????? ?
                                    if( !SavedCounter )  
                                    {                                      
                                      deviceTime = 0;
                                      Radio2Tx(spiBuffer, 16, TX_DATA, ProtocolNumber); // ????????? ? ????? Tx ??? ?????? ?????? ??????
                                      break;
                                    }
                                                                        
                                    // ???????????? ????? ? ???????
                            default:                                                                        
                                    spiBuffer[TYPE_POS]             = SavedList[SavedCounter - 1].Type;
                                    spiBuffer[ADR_POS]              = SavedList[SavedCounter - 1].Adr;
                                    *((pU32)&spiBuffer[TIME_POS])   = SavedList[SavedCounter - 1].Time;
                                    *((pU16)&spiBuffer[CONFIG_POS]) = SavedList[SavedCounter - 1].Config;
                                    *((pU16)&spiBuffer[ERRORS_POS]) = SavedList[SavedCounter - 1].Errors;
                                    *((pU16)&spiBuffer[POWLEV_POS]) = SavedList[SavedCounter - 1].PowLev;
                                    
                                    switch( SavedList[SavedCounter - 1].Type )
                                    {
                                      case IVTM_7M4_OKW:
                                      case IVTM_7M4_PRT:  messageLen = Encode_7M4( );  
                                                          break;
                                                
                                      case MAG_7:         messageLen = Encode_MAG7( ); 
                                                          break;
                                    };
                                    
                                    Radio2Tx(spiBuffer, messageLen, TX_DATA, ProtocolNumber);
                                    timeout_ans = SX1276_TimeOnAir( messageLen + 5 ) * 1000;
                        }
                        break;
/* ####################################  ????? ?????????? - ???????????? ################################# */
                        
   } // switch mode    
  } // for
}
                               
void Radio2IDLE(void)
{
  SX1276_Standby( );
  SWOFF_RX; 
    SWOFF_TX;
}
           
void Radio2Rx(U8 mode, U8 protocol)
{  
  U32   currentchannel     ;
  U8    current_BW_Setting ;
  U8    messLen;
  bool  fixLen;
  
  switch( mode )
  {
                  // ????? ?????? ???????
    case RX_DATA: currentchannel     = (U32)RadioChannelModem * CHANEL_SIZE + FREQ_CARRY;
                  current_BW_Setting = BW_MODEM + BW_Setting; 
                  break;
      
                  // ????? ?????? ??????????????
    case RX_ANS:  currentchannel     = (U32)RadioChannelRetrans * CHANEL_SIZE + FREQ_CARRY;
                  current_BW_Setting = BW_RETRANS + BW_Setting;
                  break;
    
    default: return;
  }
  
  switch(protocol)
  {
    case 0:       fixLen = true;  messLen = 7;
                  break;
                  
    case 1:       fixLen = false; messLen = 0;
                  break;                 
                  
    default: return;                  
  }

  SX1276_Standby      ( );  
  SX1276_SetChannel   ( currentchannel );
                        // bandwidth,           datarate,         coderate,    preambleLen,  timeout, fixLen, payload, crcOn, freqHopOn, hopPeriod, iqInverted, rxcontinue 
  SX1276_SetRxConfig  ( current_BW_Setting, DATARATE_Setting, CODERATE_Setting, PREAMBLELEN_Setting, 0, fixLen, messLen, true, false, 0, true, true ); 
  SX1276_Rx           ( );
  SX1276_Write1       ( REG_LR_IRQFLAGS, 0xff ); // ??????? ????? ??????????
  
  EXTI_PR      = 0xffffffff;                    //  ??????? ??? EXTI ???????
  EXTI_IMR     = (1<<15);                       //  IRQ0 enable  
  LIGHT_RX; 
    SWOFF_TX;
}
            
void Radio2Tx(pU8 buffer, U8 len, U8 rx_ntx, U8 protocol)
{
  U8    current_POWEROUT_Setting;
  U8    current_BW_Setting;
  U32   freq;
  bool  fixLen;
  
  switch( rx_ntx )
  { 
                  // ??????? ????? ? ?????? ??????
    case TX_ANS:  current_POWEROUT_Setting = 17;                                              // ???????? ????????
                  current_BW_Setting       = BW_MODEM  + BW_Setting;                          // 6...9 - 62.5...500 kHz bandwidth ????? ??????
                  freq                     = (U32)RadioChannelModem * CHANEL_SIZE + FREQ_CARRY;  // ????? ??????
                  break;
      
                  // ??????? ?????? ? ?????? ?????????????
    case TX_DATA: current_POWEROUT_Setting = (RadioPowerRetrans & 0xf) + POWEROUT_Setting;    // 2 ... 17 dBm
                  current_BW_Setting       = BW_RETRANS   + BW_Setting;                       // 6...9 - 62.5...500 kHz bandwidth ????? ?????????????
                  freq                     = (U32)RadioChannelRetrans * CHANEL_SIZE + FREQ_CARRY;  //  ????? ?????????????
                  break;
    
    default: return;
  }
  
  switch(protocol)
  {
    case 0:       fixLen = true; 
                  break;
                  
    case 1:       fixLen = false;
                  break;                 
                  
    default: return;                  
  }  

  SX1276_Standby     ( );  
  SX1276_SetChannel  ( freq );
                      // power, bandwidth, datarate, coderate, preambleLen, fixLen, crcOn, freqHopOn, hopPeriod, iqInverted 
  SX1276_SetTxConfig ( current_POWEROUT_Setting, current_BW_Setting, DATARATE_Setting, CODERATE_Setting, PREAMBLELEN_Setting, fixLen, true, false, 0, true );    
  SX1276_Send        ( buffer, len );
  SX1276_Write1      ( REG_LR_IRQFLAGS, 0xff ); // ??????? ????? ??????????
  
  EXTI_PR      = 0xffffffff;                    // ??????? ??? EXTI ???????
  EXTI_IMR     = (1<<15);                       // IRQ0 enable      
  LIGHT_TX; 
    SWOFF_RX;
} 

void SendResponse(U8 command)
{
  U16 crc = 0;
  
  OS_Delay(10);
  
  switch(command)
  {
    case ASK_LORA:  spiBuffer[0] = pre_measure.Adr; spiBuffer[1] = ASK_LORA; 
                    Radio2Tx(spiBuffer, ASK_LEN, TX_ANS, ProtocolNumber);
                    break;
                    
    case ASKTIME_LORA:  
                    if ( errors_flags.timeinvalid || errors_flags.LSEfail || errors_flags.TimeSyncReq )
                    {
                      spiBuffer[0] = pre_measure.Adr; spiBuffer[1] = ASK_LORA;
                      Radio2Tx(spiBuffer, ASK_LEN, TX_ANS, ProtocolNumber);
                    }
                    else
                    {
                      spiBuffer[0] = pre_measure.Adr; spiBuffer[1] = ASKTIME_LORA; *((pU32)&spiBuffer[2]) = Time;
                      crc = GetModbusRTUCRC(spiBuffer, ASKTIME_LEN_CRC - 2);
                      *((pU16)&spiBuffer[ASKTIME_CRC_POS]) = crc;
                      Radio2Tx(spiBuffer, ASKTIME_LEN_CRC, TX_ANS, ProtocolNumber);
                    }
                    break;                                                            
                    
    case NOASKTIME_LORA:                    
                    if ( errors_flags.timeinvalid || errors_flags.LSEfail || errors_flags.TimeSyncReq )
                    {
                      spiBuffer[0] = pre_measure.Adr; spiBuffer[1] = NOASK_LORA;
                      Radio2Tx(spiBuffer, ASK_LEN, TX_ANS, ProtocolNumber);
                    }
                    else
                    {
                      spiBuffer[0] = pre_measure.Adr; spiBuffer[1] = NOASKTIME_LORA; *((pU32)&spiBuffer[2]) = Time;
                      crc = GetModbusRTUCRC(spiBuffer, ASKTIME_LEN_CRC - 2);
                      *((pU16)&spiBuffer[ASKTIME_CRC_POS]) = crc;
                      Radio2Tx(spiBuffer, ASKTIME_LEN_CRC, TX_ANS, ProtocolNumber);
                    }
                    break;
                    
    case NOASK_LORA:
                    spiBuffer[0] = pre_measure.Adr; spiBuffer[1] = NOASK_LORA; 
                    Radio2Tx(spiBuffer, ASK_LEN, TX_ANS, ProtocolNumber);
                    break;
  }
}
 
// ? Save ??????
S8 Add2SaveList(void)
{
  if( SavedCounter >= MAX_SAVED_POS ) 
  {
    errors_flags.savedMemEnd = 1;
    return ERR1;
  }
    
  errors_flags.savedMemEnd   = 0;
  OS_Use( &SemaRAM); 
    memcpy((void *) &SavedList[SavedCounter],(void *) &pre_measure, sizeof(radio_list_struct_t));
    SavedCounter++;
  OS_Unuse( &SemaRAM);
  return NOERR;
}

// ????? ??????????? ??????? ? SavedList
S8 CheckSaveListDupl (void)
{    
  for(U16 i = 0; i < SavedCounter; i++) 
    if( (SavedList[i].Adr == pre_measure.Adr) && (SavedList[i].Time ==  pre_measure.Time) )  return ERR1;
  
  return NOERR;
}

// ????? ?????? ? CurrentList ?????????? ?? ??????
S8 device_seach(U8 adr)
{    
  for(U16 i = 0; i < CurrentCounter; i++) 
    if(CurrentList[i].Adr == adr) return i;
  
  return -1;
}

// ????????? SavedList ????? ????????????
void ProceedSaveList(void)
{ 
  // if MQTT - ?? ????????? ??????? SavedList
  if(system_flags.mqtt_on) return;
  
  // ??????????? ??????????? ??????
  U16 p = SavedCounter >> 16;
  
  if( p )
  {
    SavedCounter &= 0xffff;
    if( p >= SavedCounter ) SavedCounter = 0;
    else
    {
      memcpy((void *) &SavedList[0], (void *) &SavedList[p], (SavedCounter - p ) * sizeof(radio_list_struct_t) );
      SavedCounter -= p;
    }
  }
}

// ?????? ?? ?????? CurrentList ?? ????????
void  Check4KillTimeOut(void)
{   
  for(U8 i = 0; i < CurrentCounter; i++)
  {
    if( CurrentList[i].Time + KillTime > Time ) continue;
    else
    {
      CurrentCounter--;
      memcpy((void *) &CurrentList[i],(void *) &CurrentList[i + 1], (CurrentCounter - i) * sizeof(radio_list_struct_t) ); // ???????? ?? ??????? ??????
      memset((void *) &CurrentList[CurrentCounter], 0, sizeof(radio_list_struct_t) );                                     // ??????? ????????? ??????
    }
  }
}

void sweepCurrentBuff(void)
{
  U8  badRecord = 0;
  
  for(U16 i = 0; i < CurrentCounter; i++)
  {
    switch( CurrentList[i].Type )
    {
      case IVTM_7M4_OKW:
      case IVTM_7M4_PRT:  badRecord = Check_7M4 ( CurrentList[i] );  
                          break;
                                                
      case MAG_7:         badRecord = Check_MAG7( CurrentList[i] ); 
                          break;
                                                
      default:            badRecord = 1;
    }
    if( badRecord ) 
    {
//      RadioDebug.RAM_Error_Count++;
      CurrentCounter--;
      memcpy((void *) &CurrentList[i],(void *) &CurrentList[i + 1], (CurrentCounter - i) * sizeof(radio_list_struct_t) ); // ???????? ?? ??????? ??????
      memset((void *) &CurrentList[CurrentCounter], 0, sizeof(radio_list_struct_t) );                                     // ??????? ????????? ??????
    }
    else continue;
  }   
}

void sweepSaveBuff(void)
{
  U8  badRecord = 0;
  
  for(U16 i = 0; i < SavedCounter; i++)
  {
    switch( SavedList[i].Type )
    {
      case IVTM_7M4_OKW:
      case IVTM_7M4_PRT:  badRecord = Check_7M4 ( SavedList[i] );  
                          break;
                                                
      case MAG_7:         badRecord = Check_MAG7( SavedList[i] ); 
                          break;
                                                
      default:            badRecord = 1;
    }
    if( badRecord ) 
    {
//      RadioDebug.RAM_Error_Count++;
      SavedCounter--;
      memcpy((void *) &SavedList[i],(void *) &SavedList[i + 1], (SavedCounter - i) * sizeof(radio_list_struct_t) ); // ???????? ?? ??????? ??????
      memset((void *) &SavedList[SavedCounter], 0, sizeof(radio_list_struct_t) );                                   // ??????? ????????? ??????
    }
    else continue;
  }   
}

/* ################# ?????? ?? ???????? ####################################### */
S8 Parse_7M4 ( U8 len)
{  
  pre_measure.Tempr    = *((pS16)&spiBuffer[TEMP_POS]); 
  pre_measure.Humidy   =  spiBuffer[HUM_POS]; 
  pre_measure.Pressure =  700 + (S8)spiBuffer[PRESS_POS];
 
  if( len == MESLEN_7M4_CRC )
  {
    U16 crc = *((pU16)&spiBuffer[IVTM7M4_CRC_POS]); 
    if( crc != GetModbusRTUCRC(spiBuffer, MESLEN_7M4_CRC - 2) ) return ERR1;
  }
  else 
    if( len != MESLEN_7M4OKW )        return ERR1;
  
  if( pre_measure.Humidy > 100 )      return ERR1;
  if( pre_measure.Pressure > 828 )    return ERR1;
  if( pre_measure.Pressure < 573 )    return ERR1;
  if( pre_measure.Tempr > 1550 )      return ERR1;
  if( pre_measure.Tempr < -650 )      return ERR1;  
  if( pre_measure.Adr == 0 )          return ERR1;
  if( pre_measure.PowLev > 100 )      return ERR1;  
  
  if( (ConfigFlags & TEMP_PSE_NOISE) && ( Time & 1) )
  {
    if ( Time & 2)  pre_measure.Tempr++;
    else            pre_measure.Tempr--;
    
    if (pre_measure.Tempr > 1550) pre_measure.Tempr = 1550;
    if (pre_measure.Tempr < -650) pre_measure.Tempr = -650;
  }
  
  if( (ConfigFlags & HUM_PSE_NOISE) && ( Time & 1) )
  {
    if ( Time & 2) pre_measure.Humidy++;
    else           pre_measure.Humidy--;
    
    if (pre_measure.Humidy > 100) pre_measure.Humidy = 100;
    if (pre_measure.Humidy < 0  ) pre_measure.Humidy = 0;
  }
  
  if( ConfigFlags & IGNORE_TIMESTAMP ) 
                                  pre_measure.Time   = Time;
  
  return NOERR;  
}

S8 Check_7M4 ( radio_list_struct_t numrecord )
{   
  if( numrecord.Humidy > 100 )      return ERR1;
  if( numrecord.Pressure > 828 )    return ERR1;
  if( numrecord.Pressure < 573 )    return ERR1;
  if( numrecord.Tempr > 1550 )      return ERR1;
  if( numrecord.Tempr < -650 )      return ERR1;  
  if( numrecord.Adr == 0 )          return ERR1;
  if( numrecord.PowLev > 100 )      return ERR1;  
  if( numrecord.Time > Time )       return ERR1;
  
  return NOERR;  
}

S8 Parse_MAG7 ( U8 len )
{
  pre_measure.Tempr    = *((pS16)&spiBuffer[TEMP_POS]); 
  pre_measure.Humidy   =  spiBuffer[HUM_POS]; 
  pre_measure.Pressure =  700 + (S8)spiBuffer[PRESS_POS];  
  
  for (U8 i=0; i<6; i++)  pre_measure.Measures[i]=((pS16)&spiBuffer[MEAS_POS])[i];
  
  if( len == MESLEN_MAG7_CRC )
  {
    U16 crc = *((pU16)&spiBuffer[MAG7_CRC_POS]); 
    if( crc != GetModbusRTUCRC(spiBuffer, MESLEN_MAG7_CRC - 2) ) return ERR1;
  }
  else 
    if( len != MESLEN_MAG7 )        return ERR1;
  
  if( pre_measure.Humidy > 100 )      return ERR1;
  if( pre_measure.Pressure > 828 )    return ERR1;
  if( pre_measure.Pressure < 573 )    return ERR1;
  if( pre_measure.Tempr > 1550 )      return ERR1;
  if( pre_measure.Tempr < -650 )      return ERR1;  
  if( pre_measure.Adr == 0 )          return ERR1;
  if( pre_measure.PowLev > 100 )      return ERR1;  
  
  if( (ConfigFlags & TEMP_PSE_NOISE) && ( Time & 1) )
  {
    if ( Time & 2)  pre_measure.Tempr++;
    else            pre_measure.Tempr--;
    
    if (pre_measure.Tempr > 1550) pre_measure.Tempr = 1550;
    if (pre_measure.Tempr < -650) pre_measure.Tempr = -650;
  }
  
  if( (ConfigFlags & HUM_PSE_NOISE) && ( Time & 1) )
  {
    if ( Time & 2) pre_measure.Humidy++;
    else           pre_measure.Humidy--;
    
    if (pre_measure.Humidy > 100) pre_measure.Humidy = 100;
    if (pre_measure.Humidy < 0  ) pre_measure.Humidy = 0;
  }
  
  if( ConfigFlags & IGNORE_TIMESTAMP ) 
                                  pre_measure.Time   = Time;
  
  return NOERR;
}

S8 Check_MAG7 ( radio_list_struct_t numrecord )
{  
  if( numrecord.Humidy > 100 )      return ERR1;
  if( numrecord.Pressure > 828 )    return ERR1;
  if( numrecord.Pressure < 573 )    return ERR1;
  if( numrecord.Tempr > 1550 )      return ERR1;
  if( numrecord.Tempr < -650 )      return ERR1;  
  if( numrecord.Adr == 0 )          return ERR1;
  if( numrecord.PowLev > 100 )      return ERR1;  
  if( numrecord.Time > Time )       return ERR1;
  
  return NOERR;
}

U8 Encode_7M4( void )
{
  *((pS16)&spiBuffer[TEMP_POS]) = SavedList[SavedCounter - 1].Tempr;
  spiBuffer[HUM_POS]            = SavedList[SavedCounter - 1].Humidy;
  spiBuffer[PRESS_POS]          = (S16)SavedList[SavedCounter - 1].Pressure - 700;
  
  U16 crc = GetModbusRTUCRC(spiBuffer, MESLEN_7M4_CRC - 2);
  *((pU16)&spiBuffer[IVTM7M4_CRC_POS]) = crc;
  
  return MESLEN_7M4_CRC;
}

U8 Encode_MAG7( void )
{
  *((pS16)&spiBuffer[TEMP_POS]) = SavedList[SavedCounter - 1].Tempr;
  spiBuffer[HUM_POS]            = SavedList[SavedCounter - 1].Humidy;
  spiBuffer[PRESS_POS]          = (S16)SavedList[SavedCounter - 1].Pressure - 700;
  for (U8 i=0; i<6; i++) ((pS16)&spiBuffer[MEAS_POS])[i] = SavedList[SavedCounter - 1].Measures[i];
  
  U16 crc = GetModbusRTUCRC(spiBuffer, MESLEN_MAG7_CRC - 2);
  *((pU16)&spiBuffer[MAG7_CRC_POS]) = crc;
  return MESLEN_MAG7_CRC;
}
/* ################# ?????? ?? ???????? ####################################### */

/* -------------- */
#pragma optimize=none
U8  shift_byte_spi1(U8 byte)
{
  SPI1_DR = byte; 
  while  (!SPI1_SR_bit.RXNE); 
  return SPI1_DR; 
}

/* -------------- */
void Radio_Reset (void)
{
  RES_LOW;
  OS_Delay( 2 );
  RES_HI;
  OS_Delay( 10 );
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
