#define MAX_DATA_LEN_UART1    128
#define MAX_DATA_LEN_UART3    512
#define U1BUFLEN              (MAX_DATA_LEN_UART1 * 2 + 64)
// #define RSPISD_POINTER        ((pInt8U)&Uart1Buffer[144])
#define APB1                  1
#define APB2                  2

#define U1RS485_ON            GPIOB_BSRR = (1<<5)
#define U1RS485_OFF           GPIOB_BRR  = (1<<5)

typedef struct
{
  U32 speed;
  F32 time;
} uart_speed_stuct;

extern OS_STACKPTR U32        StackUARTS[128];
extern OS_TASK                OS_UARTS;
extern U8                     Uart1Buffer[];
extern U16                    BytesToTransfer1;

void                          UART1_ISR(void);
void                          UART2_ISR(void);
void                          UART3_ISR(void);
void                          UARTS_Task(void);
void                          TIM3_ISR(void);
void                          TIM4_ISR(void);
void                          changeRSspeed(void);

U32                           EKSIS_ASCII(pU8 buffer);
uart_speed_stuct              select_speed(U32 speed, U8 apbn);