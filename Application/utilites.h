#include <stdbool.h>

#define OLED_CS_LOW                 GPIOD_BRR=  (1<<2)
#define OLED_CS_HIGH                GPIOD_BSRR= (1<<2)
#define OLED_RES_LOW                GPIOD_BRR=  (1<<1)
#define OLED_RES_HIGH               GPIOD_BSRR= (1<<1)
#define OLED_COMMAND                GPIOD_BRR=  (1<<0)
#define OLED_DATA                   GPIOD_BSRR= (1<<0)

#define LOW_FLASH_ADR               0x8018000
#define HI_FLASH_ADR                0x8040000

#define NO_KEY_PRESSED              0x00
#define KEY_UP_SHORT                0x01
#define KEY_DOWN_SHORT              0x02
#define KEY_ENTER_SHORT             0x04
#define KEY_CANCEL_SHORT            0x08

#define BOOTMARK_BASE               1
#define ERR_BASE                    2
#define TIMESYNC_BASE               3

#define BKP_BASE                    0x40006C04

void  ParseDate                     (Int32U* t, int* s, int* mi, int* h, int* d, int* m, int* y);
U32   EncodeDate                    (U8 s, U8 mi, U8 h, U8 d, U8 m, U16 y);
U32   RTC_GetCounter                (void);
void  RTC_SetCounter                (U32 newtime);

void  hexbuffer_2_binbuffer         (pInt8U hexBuffer,pInt8U binBuffer,Int16U nBytes,Int16U Pointer);
void  binbuffer_2_hexbuffer         (pInt8U hexBuffer,pInt8U binBuffer,Int16U nBytes,Int16U Pointer);
void  binbuffer_2_binbuffer         (pInt8U pBufferSource,pInt8U pBufferDestination,Int16U nBytes);

U32                                 check_sum32                   (pInt32U pBuffer,Int32U Size);
U16                                 hex_to_int                    (Int8U pos,pInt8U pBuffer);
U16                                 char_to_hex                   (Int8U cnum);
U8                                  hex_to_char                   (Int8U pos,pInt8U pBuffer);
U8                                  check_sum8                    (pInt8U pBuffer, Int16U len);
U8                                  write_Flash                   (pU16 buffer, U32 writeAdr, U32 num);
U8                                  writeConfigFlashWithCHK       (void);

void                                Load_default_config           (void);
void                                KeyTimerISR                   (void);

void                                parse_ip(Int32U* ip, char* string);
void                                pack_ip (Int32U* ip, char* string);
void                                increment_speed(Int32U* speed);

Int8U                               errors_present();

Int8U                               get_next_device_on_air    (Int8U from);
Int8U                               get_previous_device_on_air(Int8U from);

Int8U                               get_RSSI_char(Int8U RSSI);
void                                set_mark_forbooter(bool b);
bool                                get_mark_forbooter(void);
void                                set_bkp_reg(U8 adr, U16 data);
U16                                 get_bkp_reg(U8 adr);
void                                usb_exchange(pU8 buf);
void                                Conf_ISR(void);
void                                LoadAndCheckConfig(void);

Int16U                              GetParameterValueForMODBUS(U16 parameter_number, Int8U * buffer);
Int16U                              SetParameterValueForMODBUS(U16 register_adr, U16 preset_value);

bool                                temperature_error    (radio_list_struct_t* rls);

bool                                humidity_present     (radio_list_struct_t* rls);
bool                                humidity_error       (radio_list_struct_t* rls);

bool                                pressure_present     (radio_list_struct_t* rls);
bool                                pressure_error       (radio_list_struct_t* rls);
