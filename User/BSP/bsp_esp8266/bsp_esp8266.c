
/*
*********************************************************************************************************
*                           ���ʼ۸������wifiģ�飬���ǲ�û��д���еĽӿڣ�ֻʵ����TCP��ص�
*                          ���к����������̰߳�ȫ�����Զ��߳�ʹ��ʱ��Ҫ����
*   
*********************************************************************************************************
*/


#include "bsp_esp8266.h"
#include  <stdlib.h>


/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/

static     char                   wifi_buf[RES_BUFF_LEN];                     //�洢ģ�鷵�ص���Ϣ

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static      void     BSP_ESP8266_Rst(void);
static      void     WriteCmd(char *cmd);
static      void     WriteData(uint8_t *dat, uint16_t len);
static      uint16_t ReadData(char *data, OS_TICK timeOut);




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        BSP_ESP8266_Init()
*
* Description : ��ʼ���ģ�����ӵĴ��ڣ�������Ӧ�Ĳ�����
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_ESP8266_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; // 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        //��ͨ���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;   //100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //����
	GPIO_Init(GPIOC,&GPIO_InitStructure);                // 
    GPIO_SetBits(GPIOC, GPIO_Pin_1); 
	
	//��ʼ��Ӧ�Ĵ���
	
	//BSP_UART_Init(WIFI_UART_PORT, WIFI_UART_BAUD);
	
	//ReadData(wifi_buf, 1000);                             //�ȴ���������
}



/*
*********************************************************************************************************
*                                        BSP_ESP8266_Server_Init()
*
* Description : ��wifiģ������ΪTCP��������ģʽ���������ֻ������5���豸�������ݽ���8s�������Ͽ�
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
int len_r;
void BSP_ESP8266_Server_Init(void) 
{
//	WriteCmd("ATE0");                                                  // ������ 
//	len_r = ReadData(wifi_buf, 1000);                                  // �ȴ�ģ���Ӧ
//	len_r = ReadData(wifi_buf, 1000);                                  // �ȴ�ģ���Ӧ
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "ATE:%s\r\n", wifi_buf);
//	WriteCmd("AT+CWMODE=2");                                           // ����Ϊ�ȵ�ģʽ 
//	len_r = ReadData(wifi_buf, 1000);                                  // �ȴ�ģ���Ӧ
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "CWMODE:%s\r\n", wifi_buf);
//	//len_r = ReadData(wifi_buf, 1000);    	// �ȴ�ģ���Ӧ
//	BSP_ESP8266_Rst();                                                 // ��Ҫ����
//	WriteCmd("ATE0");                                                  // ������
//    len_r = ReadData(wifi_buf, 1000);                                  // �ȴ�ģ���Ӧ	
//	len_r = ReadData(wifi_buf, 1000);                                  // �ȴ�ģ���Ӧ
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "ATE0: %s\r\n", wifi_buf);
//	//ReadData(wifi_buf, 1000);                                          // �ȴ�ģ���Ӧ
//	
//	WriteCmd("AT+CWSAP_DEF=\"ESP8266\",\"1234567890\",5,3");
//	//WriteCmd(AP_CONFIG);                                               // �����ȵ���Ϣ    
//	//ReadData(wifi_buf, 1000);
//	len_r = ReadData(wifi_buf, 1000);                                  // �ȴ�ģ���Ӧ
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", wifi_buf);
//	WriteCmd("AT+CIPMUX=1");                                           // ����������Ϊ������ģʽ
//	//ReadData(wifi_buf, 1000);                                          // �ȴ�ģ���Ӧ
//	len_r = ReadData(wifi_buf, 1000);                                  // �ȴ�ģ���Ӧ
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", wifi_buf);
//	WriteCmd(AP_SERVER_CONFIG);                                        // ����������������
//	//ReadData(wifi_buf, 1000);                                          // �ȴ�ģ���Ӧ
//	len_r = ReadData(wifi_buf, 1000);                                  // �ȴ�ģ���Ӧ
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", wifi_buf);
//	WriteCmd("AT+CIPSTO=80");                                           // �����ݽ���8s��Ͽ����������5���豸
//	//ReadData(wifi_buf, 1000);  
//	len_r = ReadData(wifi_buf, 1000);                                  // �ȴ�ģ���Ӧ
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", wifi_buf);
}



uint8_t BSP_ESP8266_Server_Read(uint8_t *data, uint8_t *id)
{
	char    lenStr[4];                                             // ���Զ�ȡ�������ܳ�999byte
	uint8_t index = 0;
	uint8_t dataLen = 0, cpyIndex = 0, readLen;
	
	readLen = ReadData(wifi_buf, 0);                               // ���õȴ�
	if (0 == readLen)
		return 0;
	
	// ��ʾ�ͻ������ӻ��߶Ͽ�
	if('0' <= wifi_buf[0] && wifi_buf[0] <= '4')
		return 0;
	
	for (index = 0; index < readLen; index++) {
		if (wifi_buf[index] == ',') {
			index++;
			*id = wifi_buf[index]-'0';
			break;
		}
	}
	
	// ��ȡ���ݵĳ���
	for (dataLen = 0; index < readLen; index++, dataLen++) {
		if (wifi_buf[index] == ',') {
			index++;
			lenStr[dataLen] = 0;                       // �����ַ���
			break;
		} else {
			lenStr[dataLen] = wifi_buf[index];         // Ϊ��������atoi��׼��
		}
	}
	
	// ��ȡ��ʵ����
	for (dataLen = atoi(lenStr), cpyIndex = 0; index < dataLen; index++, cpyIndex++) {
		data[cpyIndex] = wifi_buf[index];
	}
	
	return dataLen;
}





/*
*********************************************************************************************************
*********************************************************************************************************
**                                         LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        WriteCmd()
*
* Description : ����ATָ��Զ�׷�ӻس����У��������������Ҫ�س����н���
*
* Return(s)   : none
*
* Argument(s) : cmd  Ҫִ�е�����
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  WriteCmd(char *cmd)
{
	BSP_UART_WrStr(WIFI_UART_PORT, cmd);
	BSP_UART_WrStr(WIFI_UART_PORT, "\r\n");
}


/*
*********************************************************************************************************
*                                        WriteData()
*
* Description : ֱ�ӷ������ݣ������������κδ���
*
* Return(s)   : none
*
* Argument(s) : dat  Ҫ���͵�����
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  WriteData(uint8_t *dat, uint16_t len)
{
	BSP_UART_WrData(WIFI_UART_PORT, dat, len);
}


/*
*********************************************************************************************************
*                                        ReadData()
*
* Description : ����ģ�鷵�ص���Ϣ�������ȴ�
*
* Return(s)   : ʵ�ʶ�ȡ�����ֽڳ���
*
* Argument(s) : data     ģ�鷵�ص���Ϣ
*             
*             ��timeOut  ������Ķ�ȡ�ȴ�ʱ��
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  uint16_t  ReadData(char *data, OS_TICK timeOut) 
{
	return BSP_UART_RdData(WIFI_UART_PORT, (uint8_t*)data, WIFI_BUFF_LEN, timeOut);
}


/*
*********************************************************************************************************
*                                        BSP_ESP8266_Rst()
*
* Description : ��λWIFIģ��
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void  BSP_ESP8266_Rst(void)
{	
	//��ʼ��Ӧ�Ĵ���
	WriteCmd("AT+RST");
	
	ReadData(wifi_buf, 3000);                                                //�ȴ���λ����
}




