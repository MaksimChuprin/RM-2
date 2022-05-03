#include "defines.h"

#define WDATA_POINTER             &buf[6]
#define RDATA_POINTER             &buf[2]
#define ASK_USBANS                buf[0]
#define BYTENUM_USB               buf[1]

#define PUT_CHK_NUM_ASK(A)        { U16 i; ASK_USBANS = ASK_USBEXCH; for (BYTENUM_USB = (A), i = 0, buf[2+(A)] = 0xff; i < (A)+2; i++) buf[(A)+2] += buf[i]; }
#define PUT_CHK_NUM_NAK(A)        { U16 i; ASK_USBANS = NAK_USBEXCH; for (BYTENUM_USB = (A), i = 0, buf[2+(A)] = 0xff; i < (A)+2; i++) buf[(A)+2] += buf[i]; }
#define PUT_CHK_NUM_BUSY(A)       { U16 i; ASK_USBANS = BSY_USBEXCH; for (BYTENUM_USB = (A), i = 0, buf[2+(A)] = 0xff; i < (A)+2; i++) buf[(A)+2] += buf[i]; }
  
typedef struct
{       /* result of int divide */
  int quot;
  int rem;
} udiv_t;

/* чтение времени RTC */
#pragma optimize=none
Int32U RTC_GetCounter(void)
{
  Int32U counter;
  Int16U tmp;

  do
  {
    tmp = RTC_CNTH;
    counter = (((uint32_t)tmp << 16 ) | RTC_CNTL);
  } while(tmp != RTC_CNTH);
  return counter;
}

/* запись времени RTC */
#pragma optimize=none
void RTC_SetCounter (U32 newtime)
{
  PWR_CR_bit.DBP=       1;
  while(!RTC_CRL_bit.RTOFF);
  RTC_CRL_bit.CNF=      1;
  while(!RTC_CRL_bit.RTOFF);
  RTC_CNTL=             0;  // на случай внезапного переполнения
  while(!RTC_CRL_bit.RTOFF);
  RTC_CNTH=             newtime >> 16;
  while(!RTC_CRL_bit.RTOFF);
  RTC_CNTL=             newtime & 0xffff;    
  while(!RTC_CRL_bit.RTOFF);
  RTC_CRL_bit.CNF=      0;
  while(!RTC_CRL_bit.RTOFF);
  PWR_CR_bit.DBP=       0;
}

/* запись во внутреннюю флеш контроллера ---------------------------------------- */
Int32U check_sum32(pInt32U pBuffer, Int32U Size)
{
  Int32U    crc, i;
  
  if( !Size ) return 0x12345678;
  
  for(i = 0, crc = 0, Size /= 4; i < Size; i++)  crc += pBuffer[i];
  return crc;
}

// FlashWrite 
#pragma optimize=none
U8 write_Flash(pU16 buffer, U32 writeAdr, U32 num)
{  
  if( (writeAdr >= LOW_FLASH_ADR) && ((writeAdr + num) <= HI_FLASH_ADR) )
  { 
    FLASH_KEYR = 0x45670123;  // unlock write
    FLASH_KEYR = 0xCDEF89AB;
    
    for(U32 i= 0; i < num/2 + (num % 2 != 0 ? 1 : 0); i++, writeAdr += 2) 
    {
      if( !(writeAdr & 0x7ff) )
      { // стирание страницы
        while(FLASH_SR_bit.BSY);
        FLASH_CR_bit.STRT = 0;
        FLASH_CR_bit.PER =  1;
        while(FLASH_SR_bit.BSY);
        FLASH_AR = writeAdr;
        while(FLASH_SR_bit.BSY);
        FLASH_CR_bit.STRT = 1;
        while(FLASH_SR_bit.BSY);
        FLASH_CR_bit.STRT = 0;
        FLASH_CR_bit.PER = 0;
      }
      
      while(FLASH_SR_bit.BSY);
      FLASH_AR =  writeAdr;
      FLASH_CR_bit.PG = 1;
      *((pU16)writeAdr)= buffer[i];
      while(FLASH_SR_bit.BSY);
      FLASH_CR_bit.PG = 0;
      while(FLASH_SR_bit.BSY);
    }
    while(FLASH_SR_bit.BSY);
    FLASH_CR_bit.LOCK = 1;  // lock write  
    return 0;
  }
  return 1; 
}

U8  writeConfigFlashWithCHK(void)
{
  U8 res;
  
  U32 chk = check_sum32((pInt32U) ConfigBeginAddr, CONFIGSIZE);
  res  = write_Flash((pU16)ConfigBeginAddr, ConfigFlashBeginAddr, CONFIGSIZE);
  res += write_Flash((pU16)&chk, ConfigCheckSumAdr, sizeof(chk));
  return res;
}
  
/* запись во внутреннюю флеш контроллера ---------------------------------------- */

// таймер конфигурации
void  Conf_ISR(void)
{
  writeConfigFlashWithCHK();
  set_mark_forbooter(0);
  system_flags.start_boot = 1;
}

// сканирование keys
void scan_keys(void)
{
  static U32  keyTimer;
  static U8   keyPress, keySave;

  
  // load keys
  keyPress  = ((GPIOE_IDR & ((1<<7) + (1<<8) + (1<<9) + (1<<10))) >> 7);
  keyPress  = ~keyPress & (BIT3+BIT2+BIT1+BIT0);

  // if key pressed
  if(keyPress)
  {
    if(keyPress == keySave)
    {
      keyTimer++;      
      if( (keyTimer == 1) || (((keyTimer & 0x3) == 2) && (keyTimer > 7)) )  OS_SignalEvent(keyPress, &OS_OLED);
    }
    else 
    {
      keySave  = keyPress;
      keyTimer = 0;
    }      
  }
  // if key up
  else 
  {   
    keySave  =  0;
    keyTimer =  0;
  }
}

/* программный таймер на 0.05 с - клава + выходы */
void KeyTimerISR(void)
{
  OS_RetriggerTimer( &KeyTimer);
  
  /* скан кнопок */
  scan_keys();        
}

const U8  days_per_month[13]  = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 29 };

#if     defined(__V2XX__)
const U32 shift  = 730518;  // EKSIS 2000-1-1
#elif   defined(__V3XX__)
const U32 shift  = 719561; // UNIX 1970-1-1
#else
#error  "Version Not defined!"
#endif

Int32U EncodeDate(U8 s, U8 mi, U8 h, U8 d, U8 m, U16 y)
{       
  U32    t;

  //January and February are counted as months 13 and 14 of the previous year
  if(m <= 2)
  {
    m += 12;
    y -= 1;
  }
  //Convert years to days
  t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
  //Convert months to days
  t += (30 * m) + (3 * (m + 1) / 5) + d;
  // Unix time starts on January 1st, 1970 
  t -= shift;
  //Convert days to seconds
  t *= 86400;
  //Add hours, minutes and seconds
  t += (3600 * h) + (60 * mi) + s;

  //Return time
  return t;  
}

void ParseDate(Int32U* t, int* s, int* mi, int* h, int* d, int* m, int* y)
{    
  U32 allseconds = *t;
  U16 year;
  U8  month, day, our, minute, second, k, long_year;
  
  udiv_t        ss1;
  div_t         ss2;
  ss1.quot=   allseconds/86400;
  ss1.rem=   allseconds%86400;
  
#if     defined(__V2XX__)
  year  = 2000;
#elif   defined(__V3XX__)
  year  = 1970;
#else
#error  "Version Not defined!"
#endif
  month = 1;
  day  =  1;
  
  ss1.quot++;
  
  while(ss1.quot)
  {   
    if ((year%4==0) && ((year%400==0) || (year%100!=0))) long_year = 1;
    else                                                 long_year = 0;
    
    for(U8 i= 0; i < 12; i++)
    {
      if((i == 1) && long_year) k = 12;
      else                      k = i;
      if(ss1.quot > days_per_month[k]) ss1.quot -= days_per_month[k];
      else break;
      month++;
    }
    if(month < 13) 
    {
      day = ss1.quot;
      break;
    }
    month= 1;
    year++;
  }
  ss2= div(ss1.rem, 3600);
  our= ss2.quot;  
  minute= ss2.rem / 60;
  second= ss2.rem - minute * 60;

  *s=second;
  *mi=minute;
  *h=our;
  *d=day;
  *m=month;
  *y=year;
}

// hex conversions
Int8U  hex_to_char(Int8U pos,pInt8U pBuffer)
{
     Int8U  a,b;
     
     a= pBuffer[pos] - 48;
     if(a>9) a-= 7;
           
     b= pBuffer[pos+1] - 48;
     if(b>9) b-= 7;
           
     return((a<<4) | b);
}

Int16U   hex_to_int(Int8U pos,pInt8U pBuffer)
{
     return ((hex_to_char(pos,pBuffer)<<8) + hex_to_char(pos+2,pBuffer));
}

Int16U   char_to_hex(Int8U cnum)
{
      Int8U  a,b;
      
      a= (cnum & 0x0f);
      b= (cnum & 0xf0)>>4;
      
      if(a>9) a+= 55;
          else a+= 48;
          
      if(b>9) b+= 55;
          else b+= 48;
          
      return(a | (b<<8));
}

// check sum 
Int8U check_sum8(pInt8U pBuffer, Int16U len)
{     
  Int8U crc= 0;
  
  for(Int16U i= 0; i<len; i++) crc+= pBuffer[i];
  return crc;
}

void  hexbuffer_2_binbuffer(pInt8U hexBuffer,pInt8U binBuffer,Int16U nBytes,Int16U Pointer)
{
  for(Int16U i=0;i<nBytes;i++,Pointer+= 2) binBuffer[i]= hex_to_char(Pointer,hexBuffer);
}

void  binbuffer_2_hexbuffer(pInt8U hexBuffer,pInt8U binBuffer,Int16U nBytes,Int16U Pointer)
{
  for(Int16U i=0,codedbyte;i<nBytes;i++) 
  {
    codedbyte=  char_to_hex(binBuffer[i]);
    hexBuffer[Pointer++]= codedbyte>>8;
    hexBuffer[Pointer++]= codedbyte;
  }
}

void  binbuffer_2_binbuffer(pInt8U pBufferSource,pInt8U pBufferDestination,Int16U nBytes)
{
  for(Int16U i=0;i<nBytes;i++) pBufferDestination[i] = pBufferSource[i];
}

void parse_ip(Int32U* ip, char* string)
{
  sprintf(string, "%u.%u.%u.%u", (*ip >> 24), (*ip >> 16) & 0xFF, (*ip >> 8) & 0xFF, *ip & 0xFF);
}

void pack_ip(Int32U* ip, char* string)
{
  int b1, b2, b3, b4;
  sscanf(string, "%d.%d.%d.%d", &b1, &b2, &b3, &b4);
  
  *ip = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
}

void increment_speed(Int32U* speed)
{
  switch(*speed)
  {
    case 1200:
    *speed = 2400;
    break;

    case 2400:
    *speed = 4800;
    break;
    
    case 4800:
    *speed = 9600;
    break;
    
    case 9600:
    *speed = 19200;
    break;
    
    case 19200:
    *speed = 38400;
    break;
    
    case 38400:
    *speed = 57600;
    break;
    
    case 57600:
    *speed = 115200;
    break;
    
    case 115200:
    *speed = 1200;
    break;

    default:
    *speed = 1200;
    break;
  }
}

Int8U errors_present()
{
  Int8U result = 0;
  
  if (errors_flags.timeinvalid)   result++;
  if (errors_flags.TimeSyncReq)   result++;
  if (errors_flags.config_fail)   result++;
  if (errors_flags.wifi_fail)     result++;
  if (errors_flags.radio_fail)    result++;
  if (errors_flags.gsm_fail)      result++;
  if (errors_flags.currentMemEnd) result++;
  if (errors_flags.savedMemEnd)   result++; 
  if (errors_flags.LSEfail)       result++;
  
  return result;
}

Int8U get_next_device_on_air(Int8U from)
{
  for (Int8U i = from + 1; i < 128; i++)
    if (CurrentList[i].Adr)
      return i;
  
  for (Int8U i = 0; i <= from; i++)
    if (CurrentList[i].Adr)
      return i;
  
  return 0;
}

Int8U get_previous_device_on_air(Int8U from)
{
  for (Int8S i = from - 1; i >= 0; i--)
    if (CurrentList[i].Adr)
      return i;
  
  for (Int8S i = 127; i >= from; i--)
    if (CurrentList[i].Adr)
      return i;
  
  return 0;
}

Int8U get_RSSI_char(Int8U RSSI)
{
  switch (RSSI)
  {
    case 0:  return 0x92;
    case 1:  return 0x82;
    case 2:  return 0x93;
    case 3:  return 0x94;
    default: return 0x3f;
  }
}

/*  обмен Ексис USB */
/* check  SUM */
static bool check_CHKSUM(pU8 buf)
{
  Int8U             bytenum, command, i, check_sum;
  bool              result;
  
  command   = buf[4];
  bytenum   = buf[5];
          
  switch(command)
  {
    case WF_USB:
    case WR_USB:      
    case WI_USB:          
                    for (i = 0, check_sum = 0xff; i < (6 + bytenum); i++) check_sum += buf[i];
                    result = (check_sum != buf[6 + bytenum]) ? 1 : 0;
                    break;
                    
    case RR_USB:
    case RF_USB:      
    case RI_USB:
    case IR_USB:      
    case CR_USB:
    case CT_USB:
    case DW_USB:
    case DR_USB:
    case SS_USB:         
                    for (i = 0, check_sum = 0xff; i < 6; i++) check_sum += buf[i];
                    result = (check_sum != buf[6]) ? 1 : 0;
                    break;
                    
    case BS_USB:    result = 0;
                    break;
      
    default:        result = 1;
  }
  
  return   result;
}

#pragma optimize=none
U16 get_bkp_reg(U8 reg_num)
{
  U16  *adr, data;
  
  if( reg_num > 42 ) return 0;
  
  RCC_APB1ENR_bit.BKPEN  = 1;
  
  if( reg_num > 10)   adr  = (pU16)(BKP_BASE + (reg_num - 1) * 4 + 0x14);
  else                adr  = (pU16)(BKP_BASE + (reg_num - 1) * 4);
  data = *adr;
  
  return data;
}

#pragma optimize=none
void set_bkp_reg(U8 reg_num, U16 data)
{
  U16  *adr;
  
  if( reg_num > 42 ) return;
  
  RCC_APB1ENR_bit.BKPEN  = 1;
  PWR_CR_bit.DBP =  1;
  if( reg_num > 10)   adr  = (pU16)(BKP_BASE + (reg_num - 1) * 4 + 0x14);
  else                adr  = (pU16)(BKP_BASE + (reg_num - 1) * 4);
  *adr = data;
  PWR_CR_bit.DBP =  0;    
}

/* метки для загрузчика - установить
  b - true  :0x1234 
      false :0x0000
  true - перезапуск системы с целью обновления программы
*/
void  set_mark_forbooter(bool b)
{
  set_bkp_reg(BOOTMARK_BASE, (b) ? 0x1234 : 0x0000);
}

/* проверка метки для загрузчика
  true   - перезапуск системы с целью обновления программы
  false  - обычный перезапуск системы
*/
bool  get_mark_forbooter(void)
{  
  return ( get_bkp_reg(BOOTMARK_BASE) == 0x1234 );
}

void usb_exchange(pU8 buf)
{
  U32 addr      =  *((pU32)buf);
  U8  command   =  buf[4];
  U8  bytenum   =  buf[5];
          
  if( (bytenum > USB_MAX_DATA_LEN) || check_CHKSUM( buf )) goto fail;
      
  switch(command)
  {
    case WR_USB:  if( ((addr + bytenum) > RAMSIZE) ) goto fail;
                  OS_Use( &SemaRAM);
                    memcpy( (void *)(RamBeginAddr + addr), WDATA_POINTER, bytenum);
                    if( (RamBeginAddr + addr) == (U32)&Time )  // RTC
                    {
                      RTC_SetCounter( Time );
                      
                      errors_flags.TimeSyncReq = errors_flags.timeinvalid = 0; 
                      set_bkp_reg( ERR_BASE, 0);
                      set_bkp_reg( TIMESYNC_BASE,     Time & 0xffff);
                      set_bkp_reg( TIMESYNC_BASE + 1, Time >> 16);                                            
                    }
                    ProceedSaveList();
                    
//                    if( (RamBeginAddr + addr) == (U32)&RadioDebug.Time ) memset( (void *)((U32)&RadioDebug + sizeof(RadioDebug.Time)), 0,  sizeof(RadioDebug) -  sizeof(RadioDebug.Time));
                  OS_Unuse( &SemaRAM);
                  
                  PUT_CHK_NUM_ASK(0); 
                  break;    

    case WI_USB:  if( ((addr + bytenum) > CONFIGSIZE) ) goto fail;
                  OS_RetriggerTimer( &Config_Timer );
                  OS_Use( &SemaRAM);
                    memcpy( (void *)(ConfigBeginAddr + addr), WDATA_POINTER, bytenum);                  
                  OS_Unuse( &SemaRAM);
                           
                  PUT_CHK_NUM_ASK(0);
                  break;                                  

    case RI_USB:  if( (addr + bytenum) > CONFIGSIZE )  goto fail;
                  OS_Use( &SemaRAM);
                    memcpy( RDATA_POINTER, (void *)(ConfigBeginAddr + addr), bytenum);
                  OS_Unuse( &SemaRAM);
                  
                  PUT_CHK_NUM_ASK(bytenum);
                  break;  
                  
    case RR_USB:  if( (addr + bytenum) > RAMSIZE )  goto fail;
                  OS_Use( &SemaRAM);
                    memcpy( RDATA_POINTER, (void *)(RamBeginAddr + addr), bytenum);
                  OS_Unuse( &SemaRAM);
                  
                  PUT_CHK_NUM_ASK(bytenum);
                  break;
    
    case RF_USB:  memcpy( RDATA_POINTER, (void *)((U32)main_screen + addr),  bytenum);                        
                  PUT_CHK_NUM_ASK(bytenum);
                  break;                  
    
    case IR_USB:  bytenum = 0;
                  strcpy( (void *) &buf[bytenum+2], (void *)SerialNumber);              bytenum += strlen(SerialNumber) + 1;
                  strcpy( (void *) &buf[bytenum+2], (void *)Version);                   bytenum += strlen(Version) + 1;
                  strcpy( (void *) &buf[bytenum+2], (void *)ID);                        bytenum += strlen(ID) + 1;
                  bytenum += sprintf(&buf[bytenum+2], "VID=%x PID=%x", USBD_VID, USBD_PID) + 1;
                  
                  PUT_CHK_NUM_ASK(bytenum);
                  break;     
                  
     /* снять скриншот */
     case SS_USB: memcpy( RDATA_POINTER, (void *)((U32)main_screen + addr),  bytenum);                  
                  PUT_CHK_NUM_ASK(bytenum);       
                  break;                  
     
     case BS_USB: PUT_CHK_NUM_ASK(0);
                  system_flags.start_boot = 1;
                  set_mark_forbooter(1);
                  break;                                                     
     fail:
     default:     PUT_CHK_NUM_NAK(0);
    }    
}

U16 GetParameterValueForMODBUSHoldReg(U16 parameter_number, Int8U * buffer)
{      
  return 0;
}

#if     defined(__V2XX__)
Int16U GetParameterValueForMODBUS(U16 register_adr, Int8U * buffer) // адрес регистра приходит как АДРЕС - 1
{
  U8  adr     = register_adr / 4 + 1;
  U8  p_num   = register_adr % 4;
  S8  rec_num;
  
  rec_num = device_seach(adr);
  if( rec_num == -1 )
  {
    // нет прибора на связи
    buffer[1] = 0;
    if(p_num == 3)  buffer[0] = 0xff; // error reg
    else            buffer[0] = 0;
    return 1;    
  }
  else
  {
    switch( p_num )
    {
      case 0: buffer[1] = CurrentList[rec_num].Tempr;
              buffer[0] = CurrentList[rec_num].Tempr >> 8;
              break;
              
      case 1: buffer[1] = CurrentList[rec_num].Humidy;
              buffer[0] = CurrentList[rec_num].Humidy >> 8;
              break;

      case 2: buffer[1] = CurrentList[rec_num].Pressure;
              buffer[0] = CurrentList[rec_num].Pressure >> 8;
              break;

      case 3: buffer[1] = CurrentList[rec_num].PowLev;
              buffer[0] = CurrentList[rec_num].Errors;
              break;
    }
    
    return 0;
  }
  
  return 1;
}

Int16U SetParameterValueForMODBUS(U16 register_adr, U16 preset_value) {}

#elif   defined(__V3XX__)
Int16U GetParameterValueForMODBUS(U16 register_adr, Int8U * buffer) // адрес регистра приходит как АДРЕС - 1
{
  U8    *tbuffer;
  
  OS_Use( &SemaRAM);
  // чтение текущих записей
  if( register_adr < 2048 )
  {  
    U8 adr     = register_adr / 16 + 1;
    U8 p_num   = register_adr % 16;
    
    S8 rec_num = device_seach(adr);
    if( rec_num == -1 )
    {
      // нет прибора на связи
      buffer[0] = buffer[1] = 0;
      OS_Unuse( &SemaRAM);
      return 1;
    }
    else
    {
      tbuffer   = (pU8)((U32)&CurrentList[rec_num].Time + p_num * 2);
      buffer[0] = tbuffer[1];
      buffer[1] = tbuffer[0];
      OS_Unuse( &SemaRAM);
      return 0;
    }    
  }
  
  // чтение буфера адрес регистра приходит как АДРЕС - 1
  if( register_adr >= 9999 )
  {  
    U16 adr     = (register_adr - 9999) / 16;
    U8  p_num   = (register_adr - 9999) % 16;
    
    tbuffer   = (pU8)((U32)&SavedList[adr].Time + p_num * 2);
    buffer[0] = tbuffer[1];
    buffer[1] = tbuffer[0];
    OS_Unuse( &SemaRAM);
    return 0;
  }  
  
  // чтение errors_flags, Time, CurrentCounter, SavedCounter адрес регистра приходит как АДРЕС - 1
  if( (register_adr >= 9991) && (register_adr < 9999) )
  {    
    tbuffer   = (pU8)((U32)&errors_flags + (register_adr - 9991) * 2);
    buffer[0] = tbuffer[1];
    buffer[1] = tbuffer[0];  
    OS_Unuse( &SemaRAM);
    return 0;
  }
  
  OS_Unuse( &SemaRAM);
  return 1;
}

Int16U SetParameterValueForMODBUS(U16 register_adr, U16 preset_value) // адрес регистра приходит как АДРЕС - 1
{    
  // Time
  if( (register_adr == 9993) || (register_adr == 9994) ) 
  { 
    OS_Use( &SemaRAM);
      *(pU16)((U32)&Time + (register_adr - 9993) * 2) = preset_value;    
      if( (Time & 0xffff) > 0xfffc ) Time -= 4;                         // доп 4 секунды на корявую синхронизацию modbus, ошибка до 8 сек - терпимо    
      RTC_SetCounter( Time );    
    OS_Unuse( &SemaRAM);
    
    errors_flags.TimeSyncReq = errors_flags.timeinvalid = 0; 
    set_bkp_reg( ERR_BASE, 0);
    set_bkp_reg( TIMESYNC_BASE,     Time & 0xffff);
    set_bkp_reg( TIMESYNC_BASE + 1, Time >> 16);
    
    return 0;
  }
       
  // SavedCount
  if( register_adr == 9998 ) 
  { 
    OS_Use( &SemaRAM);
    SavedCounter |= ((U32)preset_value << 16);
    ProceedSaveList();
    OS_Unuse( &SemaRAM);
    
    return 0;
  } 
  
  return 1;
}

#else
  #error  "Version Not defined!"
#endif

#pragma optimize=none
void LoadAndCheckConfig(void)
{    
  // запуск кварца 32768 если не включен
  if ( !RCC_BDCR_bit.LSEON )
  {
    PWR_CR_bit.DBP    =   1;

    RCC_BDCR_bit.BDRST=   1;
    OS_Delay(2);
    RCC_BDCR_bit.BDRST=   0;    
    RCC_BDCR_bit.LSEBYP=  0;
    
    RCC_BDCR_bit.LSEON =   1;
    OS_Delay(5000);
    if( RCC_BDCR_bit.LSERDY ) 
    {
      RCC_BDCR_bit.RTCSEL = 1;
    }
    else
    {
      RCC_BDCR_bit.LSEON  = 0;
      RCC_BDCR_bit.RTCSEL = 2;
    }
        
    RCC_BDCR_bit.RTCEN=   1;            // разрешить  RTC
    
    while(!RTC_CRL_bit.RTOFF);          // ждем завершение записи
    RTC_CRH=              0;             
    while(!RTC_CRL_bit.RTOFF);          // ждем завершение записи
    RTC_CRL_bit.CNF=      1;            // разрешит запись конфигурации
    RTC_PRLH=             0;            // предделитель на 1 секунду
    RTC_PRLL=             32767;        // предделитель на 1 секунду     
    RTC_CNTL=             0;                 
    RTC_CNTH=             0; 
    RTC_CRL_bit.CNF=      0;            // запретить запись
    while(!RTC_CRL_bit.RTOFF);          // ждем завершение записи
    PWR_CR_bit.DBP=       0;    
    
    set_bkp_reg( ERR_BASE,          1); 
    set_bkp_reg( TIMESYNC_BASE,     0);
    set_bkp_reg( TIMESYNC_BASE + 1, 0);
  }  
    
  /* init RAM */ 
  *(pU32)&errors_flags = 0;
  U32 time = RTC_GetCounter();
  if( ( time > (Time + 60)) || ( time < Time) )   
  {    
    memset( (void *)RamBeginAddr, 0, RAMSIZE);  
  }
  
  Time = time;    
  errors_flags.TimeSyncReq  = errors_flags.timeinvalid  = (bool)(get_bkp_reg(ERR_BASE));  
  errors_flags.LSEfail      = (RCC_BDCR_bit.RTCSEL == 2) ? 1 : 0;
  
  /* загрузка конфигурации  */ 
  memset( (void *)ConfigBeginAddr, 0, CONFIGSIZE );
  if( check_sum32((pU32)ConfigFlashBeginAddr, CONFIGSIZE) != ConfigCheckSumF ) 
  { 
    for(U8 i = 0; i < 8; i++) SerialNumber[i] = '0'; 
    SerialNumber[8]     =  0;
  
    ProtocolNumber      =  0;    
    TCPIPConfig.UseDHCP =  0;
    TCPIPConfig.IP      = ((U32)192 << 24) + (168L << 16) + (1L << 8)   + 241L;   // 192.168.1.240
    TCPIPConfig.Gate    = ((U32)192 << 24) + (168L << 16) + (1L << 8)   + 1L;     // 192.168.1.1
    TCPIPConfig.Mask    = ((U32)255 << 24) + (255L << 16) + (255L << 8) + 0L;     // 255.255.255.0
  
    RsSpeed             = 115200;  
    RsAdr               = 1;
    RadioChannelModem   = 5; 
    RadioChannelRetrans = 0;
    RadioPowerRetrans   = 0;
    RadioBW             = 0x33;
    
    WiFiConfig.wifi_present = GSMConfig.gsm_present = 0;

    errors_flags.config_fail  = 1; 
  } 
  else  
  { 
    memcpy( (void *)ConfigBeginAddr, (void *)ConfigFlashBeginAddr, CONFIGSIZE);  
  }    
}

bool temperature_error(radio_list_struct_t* rls)
{
  switch (rls->Type)
  {
    case IVTM_7M4_OKW:
    case IVTM_7M4_PRT:
      return rls->Errors & BIT0;
      break;
      
    case MAG_7:
      return rls->Config & BIT1;
      break;
      
    default:
      return true;
  }
}

bool humidity_present(radio_list_struct_t* rls)
{
  switch (rls->Type)
  {
    case IVTM_7M4_OKW:
    case IVTM_7M4_PRT:
      return !(rls->Errors & BIT6);
      break;
      
    case MAG_7:
      return true;
      break;
      
    default:
      return false;
  }
}

bool humidity_error(radio_list_struct_t* rls)
{
  switch (rls->Type)
  {
    case IVTM_7M4_OKW:
    case IVTM_7M4_PRT:
      return rls->Errors & BIT1;
      break;
      
    case MAG_7:
      return rls->Config & BIT2;
      break;
      
    default:
      return true;
  }
}

bool pressure_present(radio_list_struct_t* rls)
{
  switch (rls->Type)
  {
    case IVTM_7M4_OKW:
    case IVTM_7M4_PRT:
      return rls->Errors & BIT7;
      break;
      
    case MAG_7:
      return rls->Config & BIT11;
      break;
      
    default:
      return false;
  }
}

bool pressure_error(radio_list_struct_t* rls)
{
  switch (rls->Type)
  {
    case IVTM_7M4_OKW:
    case IVTM_7M4_PRT:
      return rls->Errors & BIT2;
      break;
      
    case MAG_7:
      return rls->Errors & BIT9;
      break;
      
    default:
      return true;
  }
}
/*
bool mag7parameter_present(radio_list_struct_t* rls, int param_index)
{
  if (rls->Type != MAG_7) return false;
  if (param_index >= 6)   return false;
  
  return (rls->Config & (1 << param_index));
}

bool mag7parameter_error(radio_list_struct_t* rls, int param_index)
{
  if (rls->Type != MAG_7) return true;
  if (param_index >= 6)   return true;

  return (rls->Errors & (1 << 4 + param_index));
}

F32 mag7parameter_value(radio_list_struct_t* rls, int param_index)
{
  if (rls->Type != MAG_7) return false;
  if (param_index >= 6)   return false;
  
//          if (pdType=C_TYPE_CO2) and (pdUnits=C_UNIT_PPM) then
//                pdValue:=PWord(aData+C_OFFSET_V3_MEASURES[lcIndex])^
//              else
//                pdValue:=PWord(aData+C_OFFSET_V3_MEASURES[lcIndex])^/100;

  //return (rls->Config & (1 << param_index));
  

#define MAG7_GAS_CH4  = 0;
#define MAG7_GAS_O2   = 1;
#define MAG7_GAS_CO2  = 2;
#define MAG7_GAS_CO   = 3;
#define MAG7_GAS_NH3  = 4;
#define MAG7_GAS_H2S  = 5;
#define MAG7_GAS_NO2  = 6;
#define MAG7_GAS_SO2  = 7;

const char* MAG7_GASES[]        = {"CH<sub>4</sub>", "O<sub>2</sub>", "CO<sub>2</sub>", "CO",               "NH<sub>3</sub>",   "H<sub>2</sub>S",   "NO<sub>2</sub>",   "SO<sub>2</sub>"};
const char* MAG7_GASES_UNITS[]  = {"%",              "%",             "ppm",            "мг/м<sup>3</sup>", "мг/м<sup>3</sup>", "мг/м<sup>3</sup>", "мг/м<sup>3</sup>",  "мг/м<sup>3</sup>"};
const int   MAG7_GAS_DECIMALS[] = {2,                2,               0,                 2,                 2,                  2,                  2,                   2};

}*/
