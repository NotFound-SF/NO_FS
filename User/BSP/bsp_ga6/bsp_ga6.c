/*
*********************************************************************************************************
*			���ļ���ʹ��uart�˿ڿ���GSMģ�飬������Ҫ��Ҫ��ɷ������ȹز���
*           ��ֲ��ֻ��Ҫ�޸ĵײ㺯�� WriteCmd();ReadData();
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_ga6.h"
#include "string.h"


/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/

static     OS_SEM                 SemLock;                     //ȷ��ͬʱֻ����һ�������ܹ�������ģ��
static     char                   Res[RES_BUFF_LEN];                     //�洢ģ�鷵�ص���Ϣ

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
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
*                                        BSP_GA6_Init()
*
* Description : ��ʼ���ģ�����ӵĴ��ڣ�������Ӧ�Ĳ�����
*
* Return(s)   : DEF_OK    ������ʼ��
*               DEF_FAIL  ��ʼ��ʧ��
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_GA6_Init(void)
{
	OS_ERR      err;
	
	// ������ռ�ź���
	
	OSSemCreate((OS_SEM    *)&SemLock,                                   //���������ź���
                (CPU_CHAR  *)"GA6 SemLock", 
	            (OS_SEM_CTR ) 1,
	            (OS_ERR    *)&err);
	
	if(OS_ERR_NONE != err) {
		return DEF_FAIL;
	}
	
	//��ʼ��Ӧ�Ĵ���
	
	BSP_UART_Init(GA6_UART_PORT,GA6_UART_BAUD);
	
	ReadData(Res, 2000);                                                //�ȴ���������
	
	return DEF_OK;
}



/*
*********************************************************************************************************
*                                        BSP_GA6_GetRegSt()
*
* Description : ��ȡ��ģ���ע����Ϣ,����ȹرջ���
*
* Return(s)   : REG_NO_CARD    δ��ȷ����SIM��
*               REG_NO_SING    ���ź�
*               REG_NORMAL     SIM��ע��ɹ�
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
GA6_Reg_Status BSP_GA6_GetRegSt(void) 
{
	OS_ERR           err;
	GA6_Reg_Status   status = REG_NORMAL;
	
	OSSemPend((OS_SEM *)&SemLock,                  //��Ϊ��ȫ�ֱ���������Ҫһ���ǳɣ�����Ҫ�ź���
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);	
	
	// ȡ�����ԣ�����������������Ҫ��ȡ�����Բ�����������
	
	WriteCmd("ATE0");   
	ReadData(Res, 1000);
	
	// ��ѯ״̬
	
	WriteCmd("AT+CREG?");                          //ע��״̬��ѯָ��
	ReadData(Res, 2000);                           //��ȡģ�鷵�ص�����
		
	// �������ص�����
	switch (Res[11]) {
		case '1':
			status = REG_NORMAL;
			break;
		
		case '2':
			status = REG_NO_SING;
			break;
		
		case '3':
			status = REG_NO_CARD;
			break;
		
		case '4':
			status = REG_UNKNOWN;
			break;
		
		case '5':
			status = REG_ROAM;
			break;
	}
	
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err);  
	
	return status;
}



/*
*********************************************************************************************************
*                                        BSP_GA6_GetSigQu()
*
* Description : ��ȡ��ģ����ź�ǿ�ȣ������ź�ʱ����0�����ź�ʱ�����ź�ǿ��
*
* Return(s)   : �ź�ǿ��
*
* Caller(s)   : Application
*
* Note(s)     : �����Ƿǻ���ģʽ
*********************************************************************************************************
*/

uint8_t BSP_GA6_GetSigQu(void)
{
	uint8_t signal = 0;
	OS_ERR  err;
	
	OSSemPend((OS_SEM *)&SemLock,                      //���������������������
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);	
	
	// ȡ�����ԣ�����������������Ҫ��ȡ�����Բ�����������
	
	WriteCmd("ATE0");   
	ReadData(Res, 1000);
	
	// �Ȼ�ȡע����Ϣ
	
	WriteCmd("AT+CREG?");                              //ע��״̬��ѯָ��
	ReadData(Res, 1000);                               //��ȡģ�鷵�ص�����
	
	if ('1' == Res[11]) {                              //����Ҫע��ɹ��ټ���ź�ǿ��,�����ź�ǿ��������
		WriteCmd("AT+CSQ");                            //��ȡ�ź�ǿ��
		ReadData(Res, 1000);                           //��ȡģ�鷵�ص�����
		
		// �����ź�ǿ��
		
		if (',' != Res[9]) {                           //��ʾ�ź�ǿ��Ϊ��λ��
			signal = (Res[8]-'0')*10 + (Res[9]-'0');
		} else {
			signal = Res[8]-'0';                       //�źź���ֻ��һλ����һ�㲻����
		}
		
	}
	
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err);  
	
	return signal;
}


/*
*********************************************************************************************************
*                                        BSP_GA6_SenTextMSG()
*
* Description : �ú������ڷ���Ӣ�Ķ���
*
* Argument(s) : num       Ŀ��绰����
*             : str       ��������
*
* Return(s)   : DEF_OK    ���Ͷ��ųɹ�
*               DEF_FAIL  ���Ͷ���ʧ��
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN BSP_GA6_SenTextMSG(char *num, char *str) 
{
	uint8_t  m_len;
	
	uint8_t      n;
	char         *ptr = "AT+CMGS=\"";                  //���ںϳɵ绰����
	uint8_t      end = 0x1A;                           //��Ϣ���ͽ�����־
	OS_ERR       err;
	CPU_BOOLEAN  status = DEF_FAIL;
	
	OSSemPend((OS_SEM *)&SemLock,                      //������Դ����֤ͬʱֻ��һ�����̷��Ͷ���
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// ȡ�����ԣ�����������������Ҫ��ȡ�����Բ�����������
	
	WriteCmd("ATE0");   
	ReadData(Res, 1000);
	
	// ���ö��Ÿ�ʽΪTEXT��ʽ
	
	WriteCmd("AT+CMGF=1");   
	m_len = ReadData(Res, 2000);
	Res[m_len] = 0;
	//BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", Res);
	
	// ���ö����ַ���ʽΪGSM��ʽ
	
	WriteCmd("AT+CSCS=\"GSM\"");   
	m_len = ReadData(Res, 2000);
	Res[m_len] = 0;
	//BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", Res);
	
	// ����Ŀ��绰����
	WriteData((uint8_t *)ptr, strlen(ptr));                   //����AT+CMGS="
	WriteData((uint8_t *)num, strlen(num));                  
	WriteCmd("\"");                       
	m_len = ReadData(Res, 2000);
	Res[m_len] = 0;
	//BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", Res);
	
	// ����ʵ������
	
	WriteData((uint8_t *)str, strlen(str));
	WriteData(&end, 1);                                       //�����Դ˽���
	n = ReadData(Res, 10000);                                 //���Ͷ�����Ҫ�ȴ��ϳ�ʱ��
	
	if (0 != n) {
		
		// �����ָ�������
		Res[12] = '\0';                                       //���ص����ݲ����Ǳ�׼���ַ��� 
		
		//BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", Res);
		
		if(0 == strcmp(Res+7, "ERROR")) {                     //������\r\n+CMS ERROR:500
			status = DEF_FAIL;
		} else {
			status = DEF_OK;
		}
	}
		
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err);  
	
	
	return status;
}






/*
*********************************************************************************************************
*                                        BSP_GA6_CheckCall()
*
* Description : �ú�������Ƿ��е绰���룬��Ϊÿ������ʱ�����Ƚϳ������Ըú���ʱ���ִ��Ҳ��Ҫ1s����
*
* Argument(s) : none
*
* Return(s)   : DEF_OK    ��ʾ�е绰����
*               DEF_FAIL  ��ʾ�޵绰
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_GA6_CheckCall(void)
{
	uint8_t      n;
	OS_ERR       err;
	CPU_BOOLEAN  status = DEF_FAIL;
	
	OSSemPend((OS_SEM *)&SemLock,                      //������Դ����֤ͬʱֻ��һ�����̷��Ͷ���
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	n = ReadData(Res, 1000);                           //�ȴ���ȡ1s����
	if (0 != n) {
		
		Res[6] = '\0';
		
		if (0 == strcmp(Res+2, "RING")) {
			status = DEF_OK;
		} else {
			status = DEF_FAIL;
		}
	}
	
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err); 
	
	return status;
}


/*
*********************************************************************************************************
*                                        BSP_GA6_CheckEx()
*
* Description : ģ������Լ�⣬���UART��ģ�������Ƿ�ɹ�
*
* Argument(s) : none
*
* Return(s)   : DEF_OK    ��ʾ���ӳɹ�
*               DEF_FAIL  ��ʾ����ʧ��
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_GA6_CheckEx(void)
{
	uint8_t      n;
	OS_ERR       err;
	CPU_BOOLEAN  status = DEF_FAIL;
	
	OSSemPend((OS_SEM *)&SemLock,                      //������Դ����֤ͬʱֻ��һ�����̷��Ͷ���
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	WriteCmd("AT");
	n = ReadData(Res, 200);                           
	if (0 != n) {
		status = DEF_OK;
	}
	
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err); 
	
	return status;
}


/*
*********************************************************************************************************
*                                        BSP_GA6_CallHang()
*
* Description : �Ҷϵ绰
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_GA6_CallHang(void)
{
	OS_ERR       err;
	CPU_BOOLEAN  status = DEF_FAIL;
	
	OSSemPend((OS_SEM *)&SemLock,                      //������Դ����֤ͬʱֻ��һ�����̷��Ͷ���
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	WriteCmd("ATH");
	ReadData(Res, 600);                           
	
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err); 
	
	return status;
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
	BSP_UART_WrStr(GA6_UART_PORT, cmd);
	BSP_UART_WrStr(GA6_UART_PORT, "\r\n");
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
	BSP_UART_WrData(GA6_UART_PORT, dat, len);
}




/*
*********************************************************************************************************
*                                        ReadData()
*
* Description : ����ģ�鷵�ص���Ϣ�������ȴ�
*
* Return(s)   : none
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
	return BSP_UART_RdData(GA6_UART_PORT, (uint8_t*)data, RES_BUFF_LEN, timeOut);
}
