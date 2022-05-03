#include "defines.h"
#include "sx1276.h"
#include "SX1276Driver.h"

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
  OS_Delay( time);
}

/* -------------- */
U8 spi_write(U8 byte)
{
  return shift_byte_spi1( byte );
}

/*
U16 F433CRC16(pU8 buffer)
{
  U16 crc = 0xFFFF;
  
  for(Int8U i = 0; i < (buffer[0]); i++)
  {
    crc ^= buffer[i];
    for (int j = 0; j < 8; j++)
    {
      if (crc & 1 == 1)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc = (crc >> 1);
    }
  }
  
  return crc;
}
*/
