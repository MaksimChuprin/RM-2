#include "defines.h"
#include "IP_init.h"

#define TCP_ASK                   0x00
#define TCP_CMDERR                0x01
#define TCP_NAK                   0xFF
#define PAYLOADANS_LEN            4

#define TCP_WDATA_POINTER         &tcp_buffer[8]
#define TCP_RDATA_POINTER         &tcp_buffer[4]
#define TCP_DATA_NUM              *((pU16)&tcp_buffer[2])
#define ASK_TCPANS                tcp_buffer[0]

U16 EKSIS_WIFI_TCP(pU8 tcp_buffer)
{
  U8            command;
  U16           bytenum;
  U32           addr;  
  
  addr          = *((pU32)&tcp_buffer[0]);
  bytenum       = *((pU16)&tcp_buffer[4]);
  command       = *((pU16)&tcp_buffer[6]);
  
  switch(command)
  {
  // WR     
  case WR_USB:  if( (addr + bytenum) > RAMSIZE ) goto fail;
                OS_Use( &SemaRAM);
                  memcpy(  (void *)(RamBeginAddr + addr), (void *)TCP_WDATA_POINTER, bytenum);
                  // CLOCK
                  if( (RamBeginAddr + addr) == (U32)&Time )  // RTC
                  {
                    RTC_SetCounter( Time );
                    
                    errors_flags.TimeSyncReq = errors_flags.timeinvalid = 0; 
                    set_bkp_reg( ERR_BASE, 0);
                    set_bkp_reg( TIMESYNC_BASE,     Time & 0xffff);
                    set_bkp_reg( TIMESYNC_BASE + 1, Time >> 16);
                  }
                  ProceedSaveList();
                  
//                  if( (RamBeginAddr + addr) == (U32)&RadioDebug.Time ) memset( (void *)((U32)&RadioDebug + sizeof(RadioDebug.Time)), 0,  sizeof(RadioDebug) -  sizeof(RadioDebug.Time));
                OS_Unuse( &SemaRAM);
                ASK_TCPANS   = TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN;
                break;                
  // WI 
  case WI_USB:  if( (addr + bytenum) > CONFIGSIZE ) goto fail;
                OS_Use( &SemaRAM);
                  memcpy( (void *)(ConfigBeginAddr + addr), (void *)TCP_WDATA_POINTER, bytenum);
                  // write flash                    
                  writeConfigFlashWithCHK();
                  OS_RetriggerTimer( &Config_Timer );
                OS_Unuse( &SemaRAM);
                ASK_TCPANS   = TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN;
                break;
  // RR
  case RR_USB:  if( (bytenum > UDP_SAFE_MAXLEN) || ((addr + bytenum) > RAMSIZE) )  goto fail; 
                OS_Use( &SemaRAM);
                  memcpy( (void *)TCP_RDATA_POINTER, (void *)(RamBeginAddr + addr), bytenum);
                OS_Unuse( &SemaRAM);
                ASK_TCPANS   = TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN + bytenum; 
                break;

  // RI         
  case RI_USB:  if( (bytenum > UDP_SAFE_MAXLEN) || ((addr + bytenum) > CONFIGSIZE) ) goto fail;
                memcpy( (void *)TCP_RDATA_POINTER, (void *)(ConfigBeginAddr + addr), bytenum);
                ASK_TCPANS   = TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN + bytenum;
                break;   

  // RF - скриншорт...
  case RF_USB:  memcpy( (void *)TCP_RDATA_POINTER, (void *)(main_screen), sizeof(main_screen) );
                ASK_TCPANS   = TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN + sizeof(main_screen);
                break;                                  
                
  // IR
  case IR_USB:  bytenum = 8; 
                addr = htonl(IP_GetIPAddr(0));               
                strcpy( (void *) &tcp_buffer[bytenum], (void *)SerialNumber); bytenum += strlen(SerialNumber) + 1;
                strcpy( (void *) &tcp_buffer[bytenum], (void *)Version);      bytenum += strlen(Version) + 1;
                strcpy( (void *) &tcp_buffer[bytenum], (void *)ID);           bytenum += strlen(ID) + 1;
                memcpy((void*)   &tcp_buffer[bytenum], (void*)&addr, 4);      bytenum += 4;
                *(pU16)TCP_RDATA_POINTER = bytenum - 8;
                ASK_TCPANS   = TCP_ASK;
                TCP_DATA_NUM = PAYLOADANS_LEN + bytenum;
                break;
                        
  fail:
  default:     ASK_TCPANS   = TCP_NAK;
               TCP_DATA_NUM = PAYLOADANS_LEN;
  }
  
  return TCP_DATA_NUM;
}
