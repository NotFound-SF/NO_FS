
/*
*********************************************************************************************************
*                         ���ļ�����һ������ר�Ÿ���app�Ե�Ƭ��wifi���绰������Ϣ
*                        �����ã�����wifiģ������Ϊ����˽���app��������ָ��
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "my_app_cfg.h"

/*
*********************************************************************************************************
*                                                 TCB
*********************************************************************************************************
*/

static  OS_TCB   LocalSettingTaskTCB;

static  OS_TCB   AppTaskGUIDemoTCB;

static  OS_TCB   AppTaskWifiTCB;

static  OS_TCB   AppTaskLedTCB;

/*
*********************************************************************************************************
*                                                STACKS
*********************************************************************************************************
*/

static  CPU_STK  LocalSettingTaskStk[LOCAL_SETTING_TASK_STK_SIZE];

static  CPU_STK  AppTaskGUIDemoStk[APP_TASK_GUI_DEMO_STK_SIZE];

static  CPU_STK  AppTaskLedStk[APP_TASK_LED_STK_SIZE];

static  CPU_STK  AppTaskWifiStk[APP_TASK_WIFI_STK_SIZE];



/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskWifi      (void *p_arg);

static  void  AppTaskLed       (void *p_arg);

static  void  LocalSettingTask (void *p_arg);




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        LocalSettingTaskCreate()
*
* Description : ����������������
*
* Argument(s) : none
*
* Return(s)   : DEF_FAIL ��ʾ��������ʧ��
*             : DEF_OK   ��ʾ��������ɹ�
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  LocalSettingTaskCreate (void)
{
	OS_ERR      err;
	
	
	//����Ӧ������,emwin�Ĺٷ�ʾ������ GUIDEMO_Main
		
		OSTaskCreate((OS_TCB     *)&AppTaskGUIDemoTCB,                                        
					(CPU_CHAR   *)"GUI Demo Test", 									                     
					(OS_TASK_PTR ) SettingMainTask,									                        
					(void       *) 0,																
					(OS_PRIO     ) APP_TASK_GUI_DEMO_PRIO,					
					(CPU_STK    *)&AppTaskGUIDemoStk[0],						
					(CPU_STK_SIZE) APP_TASK_GUI_DEMO_STK_SIZE / 10,				
					(CPU_STK_SIZE) APP_TASK_GUI_DEMO_STK_SIZE,        		
					(OS_MSG_QTY  ) 0u,
					(OS_TICK     ) 0u,
					(void       *) 0,
					(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
					(OS_ERR     *)&err);															

		if(err==OS_ERR_NONE) {
//			BSP_UART_Printf(BSP_UART_ID_1, "MainTask OK");
		}
		
		
	 //�����ӽ��� LED�ӽ���
	
    OSTaskCreate((OS_TCB     *)&AppTaskLedTCB,              /* Create the LED task                                 */
	  		    (CPU_CHAR   *)"App Task Led",
			    (OS_TASK_PTR ) AppTaskLed,
			    (void       *) 0,
			    (OS_PRIO     ) APP_TASK_LED_PRIO,
			    (CPU_STK    *)&AppTaskLedStk[0],
			    (CPU_STK_SIZE) APP_TASK_LED_STK_SIZE / 10,
			    (CPU_STK_SIZE) APP_TASK_LED_STK_SIZE,
			    (OS_MSG_QTY  ) 5u,
			    (OS_TICK     ) 0u,
			    (void       *) 0,
			    (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
			    (OS_ERR     *)&err);
			 
    if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppTaskLed OK");
    }
		
		
	
	//����WIFI����
	
	OSTaskCreate((OS_TCB     *)&AppTaskWifiTCB,                                        
				(CPU_CHAR   *)"Wifi", 									                     
				(OS_TASK_PTR ) AppTaskWifi,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_WIFI_PRIO,					
				(CPU_STK    *)&AppTaskWifiStk[0],						
				(CPU_STK_SIZE) APP_TASK_WIFI_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_WIFI_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppWifi OK");
    }
	
	// ��������flash���߳�
	
	OSTaskCreate((OS_TCB    *)&LocalSettingTaskTCB,              /* Create the Sensor task                                 */
				(CPU_CHAR   *)"Local setting Task",
				(OS_TASK_PTR ) LocalSettingTask,
				(void       *) 0,
				(OS_PRIO     ) LOCAL_SETTING_TASK_PRIO,
				(CPU_STK    *)&LocalSettingTaskStk[0],
				(CPU_STK_SIZE) LOCAL_SETTING_TASK_STK_SIZE / 10,
				(CPU_STK_SIZE) LOCAL_SETTING_TASK_STK_SIZE,
				(OS_MSG_QTY  ) 0,                                // ���ڽ���Ϣ����
				(OS_TICK     ) 0u,
				(void       *) 0,
				(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);
			 
	if(err == OS_ERR_NONE) {
		
	}
	
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
*                                        LocalSettingTask()
*
* Description : ������ר�Ÿ������APP�˷��͵���������ɶԵ�Ƭ��������
*
* Argument(s) : p_arg is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : NONE
*
* Caller(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/

static uint8_t buf[180];

static  void  LocalSettingTask (void *p_arg)
{
	int i;
	OS_ERR            err;
	FLASH_DRIVER_MSG  *request = (FLASH_DRIVER_MSG *)buf;
	
	(void)p_arg;
	
	request ->option   = FLASH_OPTION_R;                       // ������
	request ->task_tcb = &LocalSettingTaskTCB;                 // ��������TCB
	request ->addr     = 0x00;                                 // Ҫ��ȡ��flash��ַ
	request ->len      = 160;                                  // ��ȡ���ֽڳ���
	
	while (DEF_ON) {
//		
//		BSP_FLASH_Drivet_Request(request);                     // ��Flash����������
//		
//		for (i = 0; i < 160; i++) {
//			BSP_UART_Printf(BSP_UART_ID_1, "%d\t", buf[sizeof(FLASH_DRIVER_MSG)+i]);
//		}
		
	
		OSTimeDlyHMSM( 0, 0, 6, 0,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
	
}



/*
*********************************************************************************************************
*                                          Wifi TASK
*
* Description : ����Wifi�շ�
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/
static  void  AppTaskWifi (void *p_arg)
{
	int len = 0;
	uint8_t data[60];
	uint8_t id = 0;
	OS_ERR  err;
	
	(void)p_arg;


	// ��wifiģ������Ϊ������ģʽ
	BSP_ESP8266_Server_Init();
	
	while(DEF_ON) {
		len = BSP_ESP8266_Server_Read(data, &id, 0);              // �����ȴ���wifiģ���ȡ����        
		
		BSP_UART_Printf(BSP_UART_ID_1, "len: %d\tid: %d\t:%s\r\n", len, id, data);
	}
	
}



/*
*********************************************************************************************************
*                                          TASK
*
* Description : 
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskLed   (void *p_arg)
{
	OS_ERR          err;
	
	(void) p_arg;
		
	/* ��ʾ���� */
	

	while(DEF_ON) {
				
		BSP_LED_Toggle(3);
		
		OSTimeDlyHMSM( 0, 0, 0, 200,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
}






