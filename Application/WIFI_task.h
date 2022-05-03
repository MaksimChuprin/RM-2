// defines
#define WIFI_RESER_LOW        GPIOD_BRR   = (1<<7)
#define WIFI_RESET_HIGH       GPIOD_BSRR  = (1<<7)


// var
extern OS_TASK                OS_WIFI;

// function
void                          WIFI_Task         (void);
void                          TIM4_ISR(void);
void                          UART2_ISR(void);