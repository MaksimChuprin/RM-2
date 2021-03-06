// defines

#ifndef __F868_TASK__
#define __F868_TASK__

#define BW_MODEM                  (RadioBW & 0x03)
#define BW_RETRANS                ((RadioBW & 0x30 ) >> 4)

#if     defined(__V2XX__)

#include "arm_comm.h"
#include <stdint.h>

#define MAX_SAVED_POS             1024
#define MAX_CURRENT_POS           128

#define FREQ_CARRY                865000000L
#define CHANEL_SIZE               1000000L

#define MESSAGE_LEN               7
#define BW_Setting                6
#define DATARATE_Setting          12
#define PREAMBLELEN_Setting       6
#define CODERATE_Setting          1
#define POWEROUT_Setting          2
#define LOW_TRESH_FREE            -80

#define REBAND_RADIO              (1<<0)
#define RECEIVE_RADIO             (1<<1)
#define TRANSMIT_RADIO            (1<<2)
#define READBUF_RADIO             (1<<3)

#define BYTENUM_POS               0

#define ADR_POS                   0
#define TEMPB1_POS                1
#define TEMPB2_POS                2
#define HUM_POS                   3
#define BAT_POS                   4
#define ERR_POS                   5
#define PRES_POS                  6

#define WRONG_ERR                 (BIT3 + BIT4 + BIT5)

// var
extern OS_TASK                    OS_F868;

// function
void                              F868_Task         (void);
void                              EXTI15_10_ISR     (void);
void                              RadioRXTimerISR   (void);
void                              Radio2Tx          (pU8 buffer, U8 len);
void                              Radio2Rx          (U16 len);
void                              Radio2IDLE        (void);
void                              ProceedSaveList   (void);
void                              Check4KillTimeOut (void);
void                              Add2SaveList      (U8 writePosition);
U8                                check_sum_spi     (pU8 buffer);
S8                                device_seach      (U8 adr);

#elif   defined(__V3XX__)

#include "arm_comm.h"
#include <stdint.h>

#define MAX_SAVED_POS             383
#define MAX_CURRENT_POS           128
#define MAXLORAMESSAGELEN         32

#define FREQ_CARRY                865000000L
#define CHANEL_SIZE               1000000L

#define BW_Setting                6
#define DATARATE_Setting          12
#define PREAMBLELEN_Setting       6
#define CODERATE_Setting          1
#define POWEROUT_Setting          2
#define LOW_TRESH_FREE            -80

#define REBAND_RADIO              (1<<0)
#define RECEIVE_RADIO             (1<<1)
#define TRANSMIT_RADIO            (1<<2)
#define READBUF_RADIO             (1<<3)

#define TYPE_POS                  0
#define ADR_POS                   1
#define TIME_POS                  2
#define CONFIG_POS                6
#define ERRORS_POS                8
#define POWLEV_POS                10
#define CHANEL_POS                11

#define ANS_ADR_POS               0
#define ANS_CMD_POS               1
#define ANS_CRC_POS               6

#define TEMP_POS                  12
#define HUM_POS                   14
#define PRESS_POS                 15
#define MEAS_POS                  16
#define IVTM7M4_CRC_POS           16
#define MAG7_CRC_POS              22

#define MODEM_MODE                0
#define DEVICE_MODE               1

#define IVTM_7M4_OKW              0
#define IVTM_7M4_PRT              1
#define MAG_7                     2

#define MESLEN_7M4OKW             16
#define MESLEN_7M4PRT             16
#define MESLEN_7M4_CRC            18
#define MESLEN_MAG7               22
#define MESLEN_MAG7_CRC           24

#define WRONG_ERR                 (BIT3 + BIT4 + BIT5)

#define NOERR                     0
#define ERR1                      1

#define ASK_LORA                  0
#define ASKTIME_LORA              1
#define NOASK_LORA                2
#define NOASKTIME_LORA            3


#define ASKTIME_CRC_POS           6
#define ASK_LEN                   2
#define NOASK_LEN                 2
#define ASKTIME_LEN               6
#define ASKTIME_LEN_CRC           8
#define NOASKTIME_LEN             6
#define NOASKTIME_LEN_CRC         8

#define TX_ANS                    0
#define TX_DATA                   1

#define RX_ANS                    0
#define RX_DATA                   1

#define DISABLE_LORA_INT          { EXTI_PR = 0xffffffff; EXTI_IMR = 0; }

// var
extern OS_TASK                    OS_F868;

// function
void                              F868_Task_1       (void);
void                              F868_Task_2       (void);
void                              EXTI15_10_ISR     (void);
void                              RadioRXTimerISR   (void);
void                              SendResponse      (U8 command);
void                              Radio2Rx(U8 mode, U8 protocol);
void                              Radio2Tx(pU8 buffer, U8 len, U8 rx_ntx, U8 protocol);
void                              Radio2IDLE        (void);
void                              ProceedSaveList   (void);
void                              Check4KillTimeOut (void);
void                              sweepCurrentBuff(void);
void                              sweepSaveBuff(void);
U8                                check_sum_spi     (pU8 buffer);
S8                                device_seach      (U8 adr);
S8                                Add2SaveList      (void);
S8                                CheckSaveListDupl (void);
S8                                Parse_7M4         (U8 len); 
S8                                Parse_MAG7        (U8 len);
U8                                Encode_7M4        ( void );
U8                                Encode_MAG7       ( void );
S8                                Check_7M4 ( radio_list_struct_t numrecord );
S8                                Check_MAG7 ( radio_list_struct_t numrecord );
#else
  #error  "Version Not defined!"
#endif

#endif /* __F868_TASK__ */