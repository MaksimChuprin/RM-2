extern OS_TASK                    OS_MQTT;
extern OS_STACKPTR U32            MQTTStack[256];

void                              MQTT_Task                   (void);
void                              ETH_IRQHandler              (void);

#define   MQTT_THR_ETH            0
#define   MQTT_THR_GSM            1
#define   MQTT_THR_WIFI           2
