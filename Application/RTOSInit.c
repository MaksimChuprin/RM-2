#include "defines.h"

/*********************************************************************
*
*       Clock frequency settings
*/
#ifndef   OS_FSYS                   /* CPU main clock frequency      */
  #define OS_FSYS 72000000uL
#endif

#ifndef   OS_PCLK_TIMER             /* Peripheral clock for timer   */
  #define OS_PCLK_TIMER OS_FSYS     /* May vary from CPU clock      */
#endif                              /* depending on CPU             */

#ifndef   OS_PCLK_UART              /* Peripheral clock for UART    */
  #define OS_PCLK_UART OS_FSYS      /* May vary from CPU clock      */
#endif                              /* depending on CPU             */

#ifndef   OS_TICK_FREQ
  #define OS_TICK_FREQ (1000)
#endif

#ifndef   OS_USE_VARINTTABLE        /* The interrupt vector table   */
  #define OS_USE_VARINTTABLE (0)    /* may be located in RAM        */
#endif

/*********************************************************************
*
*       UART settings for OSView
*       If you do not want (or can not due to hardware limitations)
*       to dedicate a UART to OSView, please define it to be -1
*       Currently the standard code enables OS_UART 1 (USART2) per default
*       and supports USART2 and 3
*/
#ifndef   OS_UART
  #define OS_UART (-1)
#endif

#ifndef   OS_BAUDRATE
  #define OS_BAUDRATE (38400)
#endif

/****** End of configuration settings *******************************/

#define OS_UART_USED ((OS_UART == 1) || (OS_UART == 2))

/*********************************************************************
*
*       Local defines (sfrs used in RTOSInit.c)
*
**********************************************************************
*/

#define SYS_INT_CTRL_STATE            (*(volatile OS_U32*)(0xE000ED04))
#define SYS_PENDSTSET                 26

#define SYSPRI1_ADDR                  (0xE000ED18)
#define SYSHND_CTRL_ADDR              (0xE000ED24)  // System Handler Control and State
#define SYSHND_CTRL                   (*(volatile OS_U32*) (SYSHND_CTRL_ADDR))

#define NVIC_SYS_HND_CTRL_MEM         (0x00010000)  // Mem manage fault enable
#define NVIC_SYS_HND_CTRL_BUS         (0x00020000)  // Bus fault enable
#define NVIC_SYS_HND_CTRL_USAGE       (0x00040000)  // Usage fault enable

#define NVIC_PRIOBASE_ADDR            (0xE000E400)
#define NVIC_ENABLE_ADDR              (0xE000E100)
#define NVIC_DISABLE_ADDR             (0xE000E180)
#define NVIC_VTOREG_ADDR              (0xE000ED08)

#define NUM_INTERRUPTS                (16+68)

#define AFIO_BASE_ADDR                (0x40010000)
#define AFIO_MAPR                     (*(volatile OS_U32*)(AFIO_BASE_ADDR + 0x04))

#define PERIPH_BASE_ADDR              ((OS_U32)0x40000000)
#define APB1PERIPH_BASE_ADDR          (PERIPH_BASE_ADDR)
#define APB2PERIPH_BASE_ADDR          (PERIPH_BASE_ADDR + 0x10000)
#define AHBPERIPH_BASE_ADDR           (PERIPH_BASE_ADDR + 0x20000)

#define RCC_BASE_ADDR                 (AHBPERIPH_BASE_ADDR + 0x1000)
#define RCC_APB2ENR                   (*(volatile OS_U32*)(RCC_BASE_ADDR + 0x18))

#define SYS_TICK_BASE_ADDR            (0xE000E010)
#define SYS_TICK_CONTROL              (*(volatile OS_U32*)(SYS_TICK_BASE_ADDR + 0x00))
#define SYS_TICK_RELOAD               (*(volatile OS_U32*)(SYS_TICK_BASE_ADDR + 0x04))
#define SYS_TICK_VALUE                (*(volatile OS_U32*)(SYS_TICK_BASE_ADDR + 0x08))
#define SYS_TICK_CALIBRATION          (*(volatile OS_U32*)(SYS_TICK_BASE_ADDR + 0x0C))
#define SYS_TICK_ENABLE_BIT           (0)
#define SYS_TICK_INT_ENABLE_BIT       (1)
#define SYS_TICK_CLK_SOURCE_BIT       (2)

#define FLASH_Latency_2               ((OS_U32)0x00000002)
#define ACR_LATENCY_Mask              ((OS_U32)0x00000038)
#define ACR_PRFTBE_Mask               ((OS_U32)0xFFFFFFEF)
#define FLASH_PrefetchBuffer_Enable   ((OS_U32)0x00000010)
#define FLASH_BASE_ADDR               ((OS_U32)0x40022000)
#define FLASH_ACR                     (*(volatile OS_U32*)(FLASH_BASE_ADDR + 0x00))

/*****  Interrupt ID numbers ****************************************/
#define ISR_ID_MPU                    (4)                // MPU fault
#define ISR_ID_BUS                    (5)                // Bus fault
#define ISR_ID_USAGE                  (6)                // Usage fault
#define ISR_ID_SYSTICK                (15)               // System Tick

#define ISR_ID_USART1                 (53)               // USART1
#define ISR_ID_USART2                 (54)               // USART2
#define ISR_ID_USART3                 (55)               // USART3

#define OS_ISR_ID_TICK                ISR_ID_SYSTICK     // We use Sys-Timer
#define OS_ISR_TICK_PRIO              (0xFF)             // Lowest priority

/****** OS timer configuration **************************************/
#define OS_TIMER_RELOAD               (OS_PCLK_TIMER / OS_TICK_FREQ - 1)
#if (OS_TIMER_RELOAD >= 0x100000000)
  #error "Systick can not be used, please check configuration"
#endif


/*********************************************************************
*
*       OS_Systick
*
* Function description
*   This is the code that gets called when the processor receives a
*   _SysTick exception. SysTick is used as OS timer tick.
*
* NOTES:
*   (1) It has to be inserted in the interrupt vector table, if RAM
*       vectors are not used. Therefore is is declared public
*/

void OS_Systick(void) 
{
  OS_EnterNestableInterrupt();
  OS_HandleTick();
  OS_LeaveNestableInterrupt();
}

/*********************************************************************
*
*       OS_InitHW()
*
*       Initialize the hardware (timer) required for the OS to run.
*       May be modified, if an other timer should be used
*/
void OS_InitHW(void) 
{
  VTOR = 0x8004000;
  RCC_APB2RSTR = RCC_APB1RSTR = RCC_CIR= 0x00000000;                              // Disable APB1&2 Peripheral Reset
  RCC_APB1ENR  = RCC_APB2ENR  = 0x00000000;                                       // Disable all APB1&2 Peripheral Clock  
  RCC_AHBENR       = 0x1ffff;                                                     // Clock ON

  RCC_CSR_bit.LSION= 1;                                                           // Вкл LSI
  RCC_CR_bit.HSION = 1;                                                           // Вкл HSI
  RCC_CFGR         = 0;                                                           // HSI как SYSCLK
  RCC_CR           = 1;                                                           // Reset HSEON, CSSON and PLLON bits
  
  RCC_CR_bit.HSEON = 1;                                                           // Запуск кварца  
  while (!RCC_CR_bit.HSERDY);                                                     // Wait till HSE is ready
  RCC_CR_bit.CSSON = 1;
  
  // настройка работы с флеш и буферизацией
  FLASH_ACR   &= ACR_LATENCY_Mask;                                                // Flash 2 wait state and Prefetch enable
  FLASH_ACR   |= FLASH_Latency_2;
  FLASH_ACR   &= ACR_PRFTBE_Mask;                                                 // Enable or disable the Prefetch Buffer
  FLASH_ACR   |= FLASH_PrefetchBuffer_Enable;
  
  // настройка частоты шин и запуск PLL-ей
  RCC_CFGR_bit.HPRE       =   0;                                                   // AHB = SYSCLK = HCLK = 72 MHz
  RCC_CFGR_bit.PPRE1      =   4;                                                   // APB1 = HCLK/2 = 36 MHz
  RCC_CFGR_bit.PPRE2      =   0;                                                   // APB2 = HCLK = = 72 MHz
  RCC_CFGR_bit.ADC_PRE    =   2;                                                   // ADCCLK=  APB2/6 = 12 MHz
  RCC_CFGR_bit.PLLSRC     =   1;                                                   // PLL1SRC= PREDIV1
  RCC_CFGR_bit.PLLMUL     =   7;                                                   // PLL1 = PREDIV1 * 9 = 72 MHz, MUL = (PLLMUL + 2)
  RCC_CFGR_bit.OTGFSPRE   =   0;                                                   // USBCLK=  PLL/1.5 = 48 MHz
  RCC_CFGR_bit.MCO        =   11;                                                  // MCO = PLL3 = 50 MHz
  RCC_CFGR2_bit.PREDIV1   =   4;                                                   // PREDIV1 = 5 = PLL2 / 5 = 8 MHz
  RCC_CFGR2_bit.PREDIV2   =   4;                                                   // PREDIV2 = 5
  RCC_CFGR2_bit.PLL2MUL   =   6;                                                   // PLL2 = in x 8 = HSE / 5 * 8 = 40 MHz
  RCC_CFGR2_bit.PLL3MUL   =   8;                                                   // PLL3 = in x 10 = HSE / 5 * 10 = 50 MHz
  RCC_CFGR2_bit.PREDIV1SRC=   1;                                                   // PREDIV1SRC = PLL2
  
  RCC_CR_bit.PLL2ON       =   1;                                                   // PLL2 start 
  RCC_CR_bit.PLL3ON       =   0;                                                   // PLL3 stop  
  
  while (!RCC_CR_bit.PLL2RDY);                                                     // Wait till PLL2 is ready
  RCC_CR_bit.PLLON        =   1;                                                   // PLL1 start 
  
  while (!RCC_CR_bit.PLLRDY);                                                      
  RCC_CFGR_bit.SW         =   2;                                                   // select PLL2->PLL1 as SYSCLK
    
  // вкючение переферийных модулей на APB1 = 36 MHz
  RCC_APB1ENR_bit.PWREN   = RCC_APB1ENR_bit.BKPEN = 1;
  RCC_APB1ENR_bit.SPI3EN  = 1;
  RCC_APB1ENR_bit.USART2EN= 1;
  RCC_APB1ENR_bit.TIM2EN  = 1; 
  RCC_APB1ENR_bit.TIM3EN  = 1; 
  RCC_APB1ENR_bit.TIM4EN  = 1; 
  
  // вкючение переферийных модулей на APB2  = 72 MHz
  RCC_APB2ENR_bit.USART1EN= 1; 
  RCC_APB2ENR_bit.SPI1EN  = 1;
  RCC_APB2ENR_bit.IOPAEN  = 1; 
  RCC_APB2ENR_bit.IOPBEN  = 1; 
  RCC_APB2ENR_bit.IOPCEN  = 1; 
  RCC_APB2ENR_bit.IOPDEN  = 1; 
  RCC_APB2ENR_bit.IOPEEN  = 1; 
  RCC_APB2ENR_bit.AFIOEN  = 1;
    
  // настройка портов
  AFIO_MAPR_bit.SWJ_CFG      = 2;                                                  // remap PoRTs JTAG-DP Disabled and SW-DP Enabled 
  AFIO_MAPR_bit.USART1_REMAP = 1;
  AFIO_MAPR_bit.USART2_REMAP = 1;
  AFIO_MAPR_bit.SPI3_REMAP   = 1;
  AFIO_MAPR_bit.ETH_REMAP    = 1;
  AFIO_MAPR_bit.MII_RMII_SEL = 1;
  
  GPIOA_ODR= 0;
  GPIOA_CRL= 0xA8A22B42;
  GPIOA_CRH= 0x4222224B;
  
  GPIOB_ODR= 0;                         
  GPIOB_CRL= 0x4A222422;
  GPIOB_CRH= 0x22BBB222;

  GPIOC_ODR= (1<<6);
  GPIOC_CRL= 0x422222B2;
  GPIOC_CRH= 0x222A2A22;

  GPIOD_ODR= 0;
  GPIOD_CRL= 0x24A22222;
  GPIOD_CRH= 0x44222444;
  
  GPIOE_ODR= (1<<7)+(1<<8)+(1<<9)+(1<<10);
  GPIOE_CRL= 0x82222222;
  GPIOE_CRH= 0x22222888;
  
  // настройка сторожевого таймера
  IWDG_KR=   0x5555;   // ключ настройки
  IWDG_PR=   6;        // 24 с таймаут
  IWDG_KR=   0x5555;   // ключ настройки
  IWDG_RLR=  0xfff;    // 
  IWDG_KR=   0xaaaa;   // сбросить перед запуском  
        
  // SPI1  - SX1276
  SPI1_CR1=           0;
  SPI1_CR1_bit.BR=    4;                    // 72/32 = 2,3 MHz
  SPI1_CR2=           0;
  SPI1_CR1_bit.SSM= SPI1_CR1_bit.SSI= 1;    // sowtware NSS
  SPI1_CR1_bit.MSTR=  1;
  SPI1_CR1_bit.SPE=   1;
  
  // SPI3 - индикатор
  SPI3_CR1=           0;
  SPI3_CR1_bit.BR=    4;                    // 36/32 = 1,125 MHz
  SPI3_CR2=           0;   
  SPI3_CR1_bit.SSM=   1;                    // sowtware NSS
  SPI3_CR1_bit.SSI=   1;
  SPI3_CR1_bit.MSTR=  1;
  SPI3_CR1_bit.SPE=   1;  
                 
  // Initialize OS timer, clock soure = core clock
  OS_IncDI();
  
  SYS_TICK_RELOAD  = OS_TIMER_RELOAD;
  SYS_TICK_CONTROL = (1 << SYS_TICK_ENABLE_BIT) | (1 << SYS_TICK_CLK_SOURCE_BIT);
  
  // Install Systick Timer Handler and enable timer interrupt
  OS_ARM_InstallISRHandler(OS_ISR_ID_TICK, (OS_ISR_HANDLER*)OS_Systick);
  OS_ARM_ISRSetPrio(OS_ISR_ID_TICK, OS_ISR_TICK_PRIO);
  OS_ARM_EnableISR(OS_ISR_ID_TICK);
  
  OS_DecRI();  
    
  set_mark_forbooter(0);
}

/*********************************************************************
*
*       Idle loop  (OS_Idle)
*
*       Please note:
*       This is basically the "core" of the idle loop.
*       This core loop can be changed, but:
*       The idle loop does not have a stack of its own, therefore no
*       functionality should be implemented that relies on the stack
*       to be preserved. However, a simple program loop can be programmed
*       (like toggeling an output or incrementing a counter)
*/
void OS_Idle(void) {     // Idle loop: No task is ready to execute
  while (1) {
  }
}

/*********************************************************************
*
*       OS_GetTime_Cycles()
*
*       This routine is required for high
*       resolution time maesurement functions.
*       It returns the system time in timer clock cycles.
*/
OS_U32 OS_GetTime_Cycles(void) {
  unsigned int t_cnt;
  OS_U32 time;
  time  = OS_Time;
  t_cnt = (OS_PCLK_TIMER/1000) - SYS_TICK_VALUE;
  if (SYS_INT_CTRL_STATE & (1 << SYS_PENDSTSET)) {    /* Missed a counter interrupt? */
    t_cnt = (OS_PCLK_TIMER/1000) - SYS_TICK_VALUE;    /* Adjust result               */
    time++;
  }
  return (OS_PCLK_TIMER/1000) * time + t_cnt;
}

/*********************************************************************
*
*       OS_ConvertCycles2us
*
*       Convert Cycles into micro seconds.
*
*       If your clock frequency is not a multiple of 1 MHz,
*       you may have to modify this routine in order to get proper
*       diagonstics.
*
*       This routine is required for profiling or high resolution time
*       measurement only. It does not affect operation of the OS.
*/
OS_U32 OS_ConvertCycles2us(OS_U32 Cycles) {
  return Cycles/(OS_PCLK_TIMER/1000000);
}



/*********************************************************************
*
*       OS interrupt handler and ISR specific functions
*
**********************************************************************
*/

#if OS_USE_VARINTTABLE
//
// The interrupt vector table may be located anywhere in RAM
//
#ifdef __ICCARM__
#if (__VER__ < 500)
#pragma segment="VTABLE"
#else
#pragma section="VTABLE"
#endif  // #if (__VER__ < 500)
#pragma data_alignment=256
__no_init void (*g_pfnRAMVectors[_NUM_INTERRUPTS])(void) @ "VTABLE";
#endif  // __ICCARM__
#ifdef __CC_ARM
extern unsigned int VTABLE$$Base;
__attribute__((section("VTABLE"), zero_init, aligned(256))) void (*g_pfnRAMVectors[_NUM_INTERRUPTS])(void);
#endif
#define VTBASE_IN_RAM_BIT             (29)
#define _RAM_START_ADDR               (0x20000000)
#endif

/*********************************************************************
*
*       OS_ARM_InstallISRHandler
*/
OS_ISR_HANDLER* OS_ARM_InstallISRHandler (int ISRIndex, OS_ISR_HANDLER* pISRHandler) {
#if OS_USE_VARINTTABLE
  OS_ISR_HANDLER*  pOldHandler;
  OS_U32 ulIdx;

  pOldHandler = NULL;
  //
  // Check whether the RAM vector table has been initialized.
  //
  if ((*(OS_U32*)NVIC_VTOREG_ADDR) != (unsigned long)g_pfnRAMVectors) {
    //
    // Copy the vector table from the beginning of FLASH to the RAM vector table.
    //
    for(ulIdx = 0; ulIdx < NUM_INTERRUPTS; ulIdx++) {
      g_pfnRAMVectors[ulIdx] = (void (*)(void))(*((volatile unsigned long *)(ulIdx * 4)));
    }
    //
    // Program NVIC to point at the RAM vector table.
    //
#ifdef __ICCARM__
    *(OS_U32*)NVIC_VTOREG_ADDR = ((OS_U32)__sfb("VTABLE") - _RAM_START_ADDR) | (1 << VTBASE_IN_RAM_BIT);
#endif
#ifdef __CC_ARM
	*(OS_U32*)NVIC_VTOREG_ADDR = ((OS_U32)&(VTABLE$$Base) - _RAM_START_ADDR) | (1 << VTBASE_IN_RAM_BIT);
#endif
  }
  //
  // Save the interrupt handler.
  //
  pOldHandler = g_pfnRAMVectors[ISRIndex];
  g_pfnRAMVectors[ISRIndex] = pISRHandler;
  return (pOldHandler);
#else
  return (NULL);
#endif
}

/*********************************************************************
*
*       OS_ARM_EnableISR
*/
void OS_ARM_EnableISR(int ISRIndex) {
  OS_DI();
  if (ISRIndex < NUM_INTERRUPTS) {
    if (ISRIndex >= 16) {
      //
      // Enable standard "external" interrupts, starting at index 16
      //
      ISRIndex -= 16;
      *(((OS_U32*) NVIC_ENABLE_ADDR) + (ISRIndex >> 5)) = (1 << (ISRIndex & 0x1F));
    } else if (ISRIndex == ISR_ID_MPU) {
      //
      // Enable the MemManage interrupt.
      //
      SYSHND_CTRL |= NVIC_SYS_HND_CTRL_MEM;
    } else if (ISRIndex == ISR_ID_BUS) {
      //
      // Enable the bus fault interrupt.
      //
      SYSHND_CTRL |= NVIC_SYS_HND_CTRL_BUS;
    } else if (ISRIndex == ISR_ID_USAGE) {
      //
      // Enable the usage fault interrupt.
      //
      SYSHND_CTRL |= NVIC_SYS_HND_CTRL_USAGE;
    } else if (ISRIndex == ISR_ID_SYSTICK) {
      //
      // Enable the System Tick interrupt.
      //
      SYS_TICK_CONTROL |= (1 << SYS_TICK_INT_ENABLE_BIT);
    }
  }
  OS_RestoreI();
}

/*********************************************************************
*
*       OS_ARM_DisableISR
*/
void OS_ARM_DisableISR(int ISRIndex) {
  OS_DI();
  if (ISRIndex < NUM_INTERRUPTS) {
    if (ISRIndex >= 16) {
      //
      // Disable standard "external" interrupts
      //
      ISRIndex -= 16;
      *(((OS_U32*) NVIC_DISABLE_ADDR) + (ISRIndex >> 5)) = (1 << (ISRIndex & 0x1F));
    } else if (ISRIndex == ISR_ID_MPU) {
      //
      // Disable the MemManage interrupt.
      //
      SYSHND_CTRL &= ~NVIC_SYS_HND_CTRL_MEM;
    } else if (ISRIndex == ISR_ID_BUS) {
      //
      // Disable the bus fault interrupt.
      //
      SYSHND_CTRL &= ~NVIC_SYS_HND_CTRL_BUS;
    } else if (ISRIndex == ISR_ID_USAGE) {
      //
      // Disable the usage fault interrupt.
      //
      SYSHND_CTRL &= ~NVIC_SYS_HND_CTRL_USAGE;
    } else if (ISRIndex == ISR_ID_SYSTICK) {
      //
      // Enable the System Tick interrupt.
      //
      SYS_TICK_CONTROL &= ~(1 << SYS_TICK_INT_ENABLE_BIT);
    }
  }
  OS_RestoreI();
}

/*********************************************************************
*
*       OS_ARM_ISRSetPrio
*
*   Notes:
*     (1) Some priorities of system handler are reserved
*         0..3 : Priority can not be set
*         7..10: Reserved
*         13   : Reserved
*     (2) System handler use different control register. This affects
*         ISRIndex 0..15
*/
int OS_ARM_ISRSetPrio(int ISRIndex, int Prio) {
  OS_U8* pPrio;
  int    OldPrio;

  OldPrio = 0;
  if (ISRIndex < NUM_INTERRUPTS) {
    if (ISRIndex >= 16) {
      //
      // Handle standard "external" interrupts
      //
      ISRIndex -= 16;                   // Adjust index
      OS_DI();
      pPrio = (OS_U8*)(NVIC_PRIOBASE_ADDR + ISRIndex);
      OldPrio = *pPrio;
      *pPrio = Prio;
      OS_RestoreI();
    } else {
      //
      // Handle System Interrupt controller
      //
      if ((ISRIndex < 4) | ((ISRIndex >= 7) && (ISRIndex <= 10)) | (ISRIndex == 13)) {
        //
        // Reserved ISR channel, do nothing
        //
      } else {
        //
        // Set priority in system interrupt priority control register
        //
        OS_DI();
        pPrio = (OS_U8*)(SYSPRI1_ADDR);
        ISRIndex -= 4;                  // Adjust Index
        OldPrio = pPrio[ISRIndex];
        pPrio[ISRIndex] = Prio;
        OS_RestoreI();
      }
    }
  }
  return OldPrio;
}

/****** Final check of configuration ********************************/

#ifndef OS_UART_USED
  #error "OS_UART_USED has to be defined"
#endif

/*****  EOF  ********************************************************/
