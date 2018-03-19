
/*
*********************************************************************************************************
*                           ���ʼ۸������wifiģ�飬���ǲ�û��д���еĽӿڣ�ֻʵ����TCP��ص�
*                          ���к����������̰߳�ȫ�����Զ��߳�ʹ��ʱ��Ҫ����
*   
*********************************************************************************************************
*/


#include "bsp_esp8266.h"
#include  <string.h>
#include  <stdlib.h>


/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/

static     char                   wifi_buf[WIFI_BUFF_LEN];                     //�洢ģ�鷵�ص���Ϣ

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

// �ú����ṩһ�����ݵ���ʱ���ɳ�ʼ���������ã���֤ÿ���ֶ���λ��
// wifiģ�鶼�������������为�����͸�λ����һ��ʱ��
static void init_delay(uint8_t count)
{
	uint32_t time;
	
	
	while (count--) {
		time = 0x2FFFF;
		while (time--);
	}
}


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
	
	// ��������ʱ��
	RCC_AHB1PeriphClockCmd(WIFI_EN_GPIO_RCC, ENABLE);
	
	// ģʽ����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        //��ͨ���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;    
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //����
	
	// ����ʹ�����ţ�����ʹ��ģ��
	GPIO_InitStructure.GPIO_Pin = WIFI_EN_PIN;             
	GPIO_Init(WIFI_EN_GPIO_PORT, &GPIO_InitStructure);  

	// ��֤ÿ���ֶ���λ���ܵõ�����
	GPIO_ResetBits(WIFI_EN_GPIO_PORT, WIFI_EN_PIN); 
	init_delay(80);
	GPIO_SetBits(WIFI_EN_GPIO_PORT, WIFI_EN_PIN);  
	ReadData(wifi_buf, 1000);                             //�ȴ���������    
	init_delay(120);
	
	// ��ʼ������
	BSP_UART_Init(WIFI_UART_PORT, WIFI_UART_BAUD);
	init_delay(2);
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

void BSP_ESP8266_Server_Init(void) 
{
	WriteCmd("ATE0");                                                  // ������ 
	ReadData(wifi_buf, 1000);                                          // �ȴ�ģ���Ӧ
	
	WriteCmd("AT+CWMODE=2");                                           // ����Ϊ�ȵ�ģʽ 
	ReadData(wifi_buf, 1000);                                          // �ȴ�ģ���Ӧ
	
	BSP_ESP8266_Rst();                                                 // ��Ҫ����
	WriteCmd("ATE0");                                                  // ������
	ReadData(wifi_buf, 1000);                                          // �ȴ�ģ���Ӧ
	
	// �����ȵ�
	WriteCmd(AP_CONFIG);
	ReadData(wifi_buf, 1000);                                          // �ȴ�ģ���Ӧ
	
	WriteCmd("AT+CIPMUX=1");                                           // ����������Ϊ������ģʽ
	ReadData(wifi_buf, 1000);                                          // �ȴ�ģ���Ӧ
	
	WriteCmd(AP_SERVER_CONFIG);                                        // ����������������
	ReadData(wifi_buf, 1000);                                          // �ȴ�ģ���Ӧ
	
	WriteCmd("AT+CIPSTO=8");                                           // �����ݽ���8s��Ͽ����������5���豸
	ReadData(wifi_buf, 1000);                                          // �ȴ�ģ���Ӧ
}


/*
*********************************************************************************************************
*                                        BSP_ESP8266_Server_Read()
*
* Description : ģ����Ϊ�������Ķ����ݺ����������������������豸���ӻ��߶Ͽ�ʱ�᷵��0(����ʱҪ��֤�ڷ�����ģʽ)
*
* Argument(s) : data�洢��ȡ�������ݣ�id��ͨ���豸�ı��[0,4]
*
* Return(s)   : 0 ��ʾ���豸���ӻ��߶Ͽ���n��ʾ��ȡ�������ݳ���
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
uint16_t BSP_ESP8266_Server_Read(uint8_t *data, uint8_t *id, OS_TICK timeout) 
{
	char     lenStr[5];                                             // ���Զ�ȡ�������ܳ�9999byte
	uint16_t index = 0;                                             // ����
	uint16_t dataLen = 0, cpyIndex = 0, readLen;
	
	readLen = ReadData(wifi_buf, timeout);                         // ���õȴ�
	if (0 == readLen)
		return 0;
	
	// ��ʾ�ͻ������ӻ��߶Ͽ�,��ʵ�������ӻس��ӻ��п�ʼ
	if(wifi_buf[0] != 0x0D || wifi_buf[1] != 0x0A || wifi_buf[2] != '+' || wifi_buf[3] != 'I')
		return 0;
	
	// ȡ�����ӵ�ID����ΧΪ0-4��ֱ��ƫ����Ϊǰ�����ݹ̶�
	*id = wifi_buf[7]-'0';
	
	// ȡ����Ч���ݳ��ȣ�ֱ��ƫ����Ϊǰ�����ݹ̶�
	for (index = 9, dataLen = 0; ':' != wifi_buf[index]; index++, dataLen++) {
		lenStr[dataLen] = wifi_buf[index];
	}
	index++;                                                      // ʹ��ָ����ʵ����
	lenStr[dataLen] = 0;                                          // �����ַ���,Ϊ��������atoi��׼��
	
	// ��ȡ��ʵ����
	for (dataLen = atoi(lenStr), cpyIndex = 0; cpyIndex < dataLen; index++, cpyIndex++) {
		data[cpyIndex] = wifi_buf[index];
	}
	
	return dataLen;
}


/*
*********************************************************************************************************
*                                        BSP_ESP8266_Server_Write()
*
* Description : ģ����Ϊ��������д���ݺ���
*
* Argument(s) : data�洢��ȡ�������ݣ�id��ͨ���豸�ı��[0,4],len��Ҫ���͵ĳ������Ϊ2048
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
void BSP_ESP8266_Server_Write(uint8_t *data, uint16_t len, uint8_t id)
{
	char send_len[6];                            // ����ܽ���2048,Ӳ������ 
	char send_cmd[26] = "AT+CIPSEND= ,";         // ����ǰ�Ŀո���ʡ��

	// ��Ϊid��Χ��0-4
	send_cmd[11] = id+'0';
	
	// ������ת��Ϊ�ַ���
	utoa(len, send_len); 
	// ƴ���ַ���
	strcat(send_cmd, send_len);
	
	// ��������
	WriteCmd(send_cmd);                                        
	ReadData(wifi_buf, 1000);                     // �ȵ�wifiģ��׼���� 
	
	// �����������ݣ����ûس��ӻ���
	WriteData(data, len);
	ReadData(wifi_buf, 1000);                     // �ȵ�wifiģ����Ӧ 
}


/*
*********************************************************************************************************
* �ú������޷���32λ����ת��Ϊ�ַ���
*
* Note(s)     : �뱣֤str�㹻����dat
*********************************************************************************************************
*/

void utoa(uint32_t dat, char *str)
{
    char    *tp;
    char    tmp_buf[12];
    uint8_t i;

    tp = tmp_buf;

    // ��֤dat = 0Ҳ�ܱ�ת��
    while (dat || tp == tmp_buf) {
        i   = dat % 10;
        dat = dat / 10;
        *tp++ = i + '0';
    }

    // ����˳��
    while (tp > tmp_buf) {
        *str++ = *--tp;
    }

    // �ַ���������־
    *str = 0;
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
	ReadData(wifi_buf, 1000);                                  // �ȴ�ģ���Ӧ
	init_delay(120);
}



