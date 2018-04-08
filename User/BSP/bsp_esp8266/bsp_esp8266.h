
#ifndef    __BSP_ESP8266_H
#define    __BSP_ESP8266_H


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "includes.h"
#include "stm32f4xx.h"
#include "bsp_uart.h"  


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define         AP_CONFIG                           "AT+CWSAP_CUR=\"STM32_Device\",\"12345678\",5,3"
#define         AP_SERVER_CONFIG                    "AT+CIPSERVER=1,2046"
#define         CLIENT_CONFIG                       "AT+CIPSTART=\"TCP\",\"192.168.1.102\",2048"

#define         REAL_SERVER_ADDR                                                // ������������ַ
#define         WIFI_UART_PORT                      BSP_UART_ID_3 
#define         WIFI_UART_BAUD                      115200
#define         WIFI_BUFF_LEN                       86                          // APP����wifi��Ϣ���62byte,ReadData����ȡ�ֽ���


// ʹ����������
#define         WIFI_EN_GPIO_PORT                        GPIOC
#define         WIFI_EN_PIN                              GPIO_Pin_1
#define         WIFI_EN_GPIO_RCC                         RCC_AHB1Periph_GPIOC      



/*
*********************************************************************************************************
*                                               FUNCTIONS
*********************************************************************************************************
*/

void     BSP_ESP8266_Init(void);
void     BSP_ESP8266_Server_Init(void);
void     utoa(uint32_t dat, char *str);
void     BSP_ESP8266_Server_Write(uint8_t *data, uint16_t len, uint8_t id);
uint16_t BSP_ESP8266_Server_Read(uint8_t *data, uint8_t *id, OS_TICK timeout);

void       BSP_ESP8266_Client_Init(void);
uint8_t    BSP_ESP8266_WIFI_connect(char *ssid, char *pwd);
uint8_t    BSP_ESP8266_WIFI_connect_status(void);
uint8_t    BSP_ESP8266_connect_server(void);
uint16_t   BSP_ESP8266_Client_Read(uint8_t *data, OS_TICK timeout); 
void       BSP_ESP8266_Client_Write(uint8_t *data, uint16_t len);





#endif   /* __BSP_ESP8266_H */





