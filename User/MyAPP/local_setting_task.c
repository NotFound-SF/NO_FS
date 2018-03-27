
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

#include "info_manager.h"     
#include "my_app_cfg.h"
#include  "user_info.h"
#include  "stdlib.h"

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

static  CPU_STK  *LocalSettingTaskStk;

static  CPU_STK  *AppTaskGUIDemoStk;

static  CPU_STK  *AppTaskLedStk;

static  CPU_STK  *AppTaskWifiStk;




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
*                                       LOCAL DEFINES
*********************************************************************************************************
*/

#define    SETTING_TASK_Q_SIZE     4                   // �����̵߳���Ϣ���г��� 

// APP --> MCU
#define    ADD_PHONE_NUM           0x63
#define    ADD_WIFI_INFO           0x60
#define    DEL_PHONE_NUM           0x61
#define    CLAEN_PHONE_NUM         0x62
#define    REQUEST                 0x12


typedef enum{
	Save_Option = 0x00,
	Get_Option = 0x01
}Option_Type;



/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_MEM                  MyPartition;

static  InfoStruct              *user_info;                                    // ʹ�ø��ڴ�������Flash�е��û�����

static  CPU_INT08U              *MyPartitionStorage;                           // ������Ϣ���д�����Ϣ    

   


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
	
	// malloc�����ڴ�ռ�
	
	user_info = (InfoStruct*)malloc(sizeof(InfoStruct));
	MyPartitionStorage = (CPU_INT08U*)malloc(SETTING_TASK_Q_SIZE*USER_INFO_SIZE*sizeof(CPU_INT08U));
	
	LocalSettingTaskStk = (CPU_STK*)malloc(LOCAL_SETTING_TASK_STK_SIZE * sizeof(CPU_STK));
	AppTaskGUIDemoStk = (CPU_STK*)malloc(APP_TASK_GUI_DEMO_STK_SIZE * sizeof(CPU_STK));
	AppTaskLedStk = (CPU_STK*)malloc(APP_TASK_LED_STK_SIZE * sizeof(CPU_STK));
	AppTaskWifiStk = (CPU_STK*)malloc(APP_TASK_WIFI_STK_SIZE * sizeof(CPU_STK));

	// �����ڴ��������ռ�
	OSMemCreate(&MyPartition, 
				 "MyPartition", 
				 MyPartitionStorage, 
				 SETTING_TASK_Q_SIZE, 
				 USER_INFO_SIZE, 
				 &err);
	
	
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
			    (OS_MSG_QTY  ) 0u,
			    (OS_TICK     ) 0u,
			    (void       *) 0,
			    (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
			    (OS_ERR     *)&err);
			 
    if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppTaskLed OK");
    }
		
		
	//����WIFI�շ�����
	
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
				(OS_MSG_QTY  ) SETTING_TASK_Q_SIZE,             // �ڽ���Ϣ���г���
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
	uint8_t len = 0;
	uint8_t data[60];
	uint8_t id = 0;
	uint8_t *msg;
	uint8_t *source = (uint8_t*)user_info;
	OS_ERR  err;
	uint16_t index;
	Option_Type opt;
	
	
	(void)p_arg;
	
	// �ֽ�Flash�е����ݶ����ڴ�
	get_user_info(user_info, &AppTaskWifiTCB); 
	
	
	// ��wifiģ������Ϊ������ģʽ
	BSP_ESP8266_Server_Init();
	
	while(DEF_ON) {

		len = BSP_ESP8266_Server_Read(data, &id, 0);              // �����ȴ���wifiģ���ȡ����    
		
		if (len > 0) {
			// ����APP�˵�������Ϣ
			switch (data[0]) {                                        // ���ֽڱ�ʾ��������
				case ADD_PHONE_NUM:
					opt = Save_Option;                                // ��ʾһ��д����
					phone_add(user_info, data+2, data[1]);
					break;
				
				case ADD_WIFI_INFO:
					opt = Save_Option;                                // ��ʾһ��д����
					wifi_setSSID(user_info, data+3, data[1]);        // ���wifi�ȵ���
					wifi_setPWD(user_info, data+3+data[1], data[2]); // ���wifi������Ϣ
					break;
				
				case DEL_PHONE_NUM:
					opt = Save_Option;                                // ��ʾһ��д����
					phone_del(user_info, data[1]);
					break;
				
				case CLAEN_PHONE_NUM:
					opt = Save_Option;                                // ��ʾһ��д����
					phone_clean(user_info);
					break;
				
				case REQUEST:                                         // ��ʾ������Ϣ
					opt = Get_Option;                                 // ��ʾ�ظ�����
					BSP_ESP8266_Server_Write(source, sizeof(InfoStruct), id);  
					break;
			}
			
			// ����APP ������������Ӧ���õĲ���
			
			if (Save_Option == opt)  {
				
				// �����ڴ�ռ�
		
				msg = (uint8_t*)OSMemGet(&MyPartition, &err);
				
				// �������ռ�ɹ���ִ�����ò���
				
				if (err == OS_ERR_NONE) {
					
					// �������ݵ�������ڴ�ռ�
					
					for (index = 0; index < USER_INFO_SIZE; index++) {
						*(msg+index) = *(source+index);
					}
					
					// ���������񷢲��ź���
					
					OSTaskQPost((OS_TCB      *) &LocalSettingTaskTCB,           // ����������
								(void        *) msg,                            // ��Ϣ����
								(OS_MSG_SIZE  ) USER_INFO_SIZE,                 // ��Ϣ����
								(OS_OPT       ) OS_OPT_POST_FIFO,               // FIFO����
								(OS_ERR      *) &err);
				} else {
					// ���벻���ռ䲻���κβ���
				}
			} 

		}
		
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

static  void  LocalSettingTask (void *p_arg)
{
	OS_ERR            err;
	OS_MSG_SIZE       msg_size;
	InfoStruct       *info_dat;
	
	(void)p_arg;
	
	while (DEF_ON) {
		
		// ���õ������ȴ���Ϣ����
		info_dat = OSTaskQPend((OS_TICK      ) 0,
							   (OS_OPT       ) OS_OPT_PEND_BLOCKING,
							   (OS_MSG_SIZE *) &msg_size,
							   (CPU_TS      *) NULL,
							   (OS_ERR      *) &err);
		
		// ���ռ��е�����д��Flash,�����ȴ���д���
		save_user_info(info_dat, &LocalSettingTaskTCB);
		
		// �ͷ���Ϣ���еĿռ�
		OSMemPut(&MyPartition,
				 info_dat, 
				 &err);
	}
	
}



