/*
*********************************************************************************************************
*                                      ���ļ�ʹ��Ƭ�������ڳ���ģʽ
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#include "DIALOG.h"
#include "includes.h"
#include "bsp_power_switch.h" 
#include "bsp_light.h"
#include "bsp_mq2.h"
#include "bsp_rain.h"
#include "my_app_cfg.h"
#include  "user_info.h"
#include  "stdlib.h"
#include  "string.h"
#include  "bsp_ga6.h"
#include  "bsp_buzzer.h"


extern  OS_TCB   AppTaskStartTCB;                                           //��ʼ����������ƿ�


/*
*********************************************************************************************************
*                                                 TCB
*********************************************************************************************************
*/

static  OS_TCB   AppTaskGUIDemoTCB;

static  OS_TCB   AppTaskTouchTCB;

static  OS_TCB   AppTaskLedTCB;

static  OS_TCB   AppTaskSensorTCB;   

static  OS_TCB   AppTaskMotorTCB;

static  OS_TCB   AppTaskWifiTCB;

static  OS_TCB   AppTaskHandleTCB;

static  OS_TCB   AppTaskCollectionTCB;

static  OS_TCB   AppTaskFireTCB;

static  OS_TCB   AppTaskBuzzerTCB;




/*
*********************************************************************************************************
*                                                STACKS
*********************************************************************************************************
*/

static  CPU_STK  *AppTaskGUIDemoStk;                       // �ÿռ��ڶ��Ϸ���

static  CPU_STK  *AppTaskTouchStk;                         // �ÿռ��ڶ��Ϸ���

static  CPU_STK  *AppTaskWifiStk;                          // �ÿռ��ڶ��Ϸ���           

static  CPU_STK  AppTaskLedStk[APP_TASK_LED_STK_SIZE];

static  CPU_STK  AppTaskMotorStk[APP_TASK_MOTOR_STK_SIZE];

static  CPU_STK  AppTaskSensorStk[APP_TASK_SENSOR_STK_SIZE];

static  CPU_STK  AppTaskHandleStk[APP_TASK_HANDLE_STK_SIZE];

static  CPU_STK  AppTaskFireStk[APP_TASK_FIRE_STK_SIZE];

static  CPU_STK  AppTaskBuzzerStk[APP_TASK_BUZZER_STK_SIZE];

static  CPU_STK  AppTaskCollectionStk[APP_TASK_COLLECTION_STK_SIZE];






/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskSensor     (void *p_arg);

static  void  AppTaskLed        (void *p_arg);

static  void  AppTaskTouch      (void *p_arg);

static  void  AppTaskMotor      (void *p_arg);

static  void  AppTaskWifi       (void *p_arg);

static  void  AppTaskHandle     (void *p_arg);

static  void  AppTaskFire       (void *p_arg);

static  void  AppTaskCollectin  (void *p_arg);

static  void  AppTaskBuzzer     (void *p_arg);



/*
*********************************************************************************************************
*                                       LOCAL DEFINES
*********************************************************************************************************
*/

#define     DEVICE_ID                     12345         // Ӳ���豸ΨһID
#define     ALIVE_TIME_OUT                2000          // ��λΪʱ�ӽ�������
 





/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/


// ���е�״̬0x0F��ʾ��״̬������0x00��ʾ״̬������

typedef struct {
	uint32_t user_id;                      // �û�ID
	uint32_t device_id;                    // �豸ID
	uint8_t  operation;                    // ������ʽ��0x0F��ʾ���ã�0x00��ʾ����
	uint8_t  warning_flag;                 // �����־��0x0F��ʾ�������֣�0x00��ʾδ����
	uint8_t  led_switch;                   // led�ƿ��ر�־��0x0F��ʾ���ƣ�0x00��ʾ�ص�
	uint8_t  led_auto;                     // led�ƹ�ر�־��0x0F��ʾ��أ�0x00��ʾ�ֿ�
	uint8_t  power_switch;                 // ��Դ���أ�0x0F��ʾ�򿪼̵�����0x00��ʾ�رռ̵���
	uint8_t  rain_status;                  // ����״̬��0x0F��ʾ���꣬0x00��ʾ����
	uint8_t  window;                       // ���ش��Ƕ�
	uint8_t  window_auto;                  // �����Զ��ش���0x0F��ʾ�Զ��ش���0x00��ʾ���Զ��ش�
	float    temperature;                  // �¶�
	float    current;                      // ����
} Local_status;




static       OS_SEM               StatusSemLock;          // ���ڶ�ռ����״̬ȫ�ֱ���  

static       Local_status         local_status;           // ȫ��״̬

static       uint8_t              server_flag = 0x00;     // ������������״̬         

static       InfoStruct           *user_info;             // ʹ�ø��ڴ�������Flash�е��û�����

static       Light_Level          light_state;            // �����������ɼ�����ʹ�������������ǿ��ˮƽ

static       uint8_t              fire_flag = 0;          // ������ֱ�����ͻᱻ��λһ



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        NormalWorkingTaskCreate()
*
* Description : ��������ģʽ����
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


CPU_BOOLEAN  NormalWorkingTaskCreate (void)
{
	OS_ERR      err;
	
	// malloc�����ڴ�ռ�
	
	user_info = (InfoStruct*)malloc(sizeof(InfoStruct));
	AppTaskGUIDemoStk = (CPU_STK*)malloc(APP_TASK_GUI_DEMO_STK_SIZE * sizeof(CPU_STK));
	AppTaskTouchStk = (CPU_STK*)malloc(APP_TASK_TOUCH_STK_SIZE * sizeof(CPU_STK));
	AppTaskWifiStk = (CPU_STK*)malloc(APP_TASK_WIFI_STK_SIZE * sizeof(CPU_STK));
	
	
	// ������ռ�ź���
	
	OSSemCreate((OS_SEM    *)&StatusSemLock,                                 //���������ź���
                (CPU_CHAR  *)"Status SemLock", 
	            (OS_SEM_CTR ) 1,                                             // �����ź���                             
	            (OS_ERR    *)&err); 
	
	// ���򵥵ĳ�ʼ��
	memset(&local_status, 0x00, sizeof(Local_status));
	
	// �Ƚ�Flash�е����ݶ����ڴ�
	get_user_info(user_info, &AppTaskStartTCB); 
	
	
	//����Ӧ������,emwin�Ĺٷ�ʾ������ GUIDEMO_Main
		
	OSTaskCreate((OS_TCB     *)&AppTaskGUIDemoTCB,                                        
				(CPU_CHAR   *)"GUI Normal", 									                     
				(OS_TASK_PTR ) MainTask,									                        
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
	
	//��������������߳�

	OSTaskCreate((OS_TCB     *)&AppTaskTouchTCB,              /* Create the LED1 task                                 */
				(CPU_CHAR   *)"App Task Touch",
				(OS_TASK_PTR ) AppTaskTouch,
				(void       *) 0,
				(OS_PRIO     ) APP_TASK_TOUCH_PRIO,
				(CPU_STK    *)&AppTaskTouchStk[0],
				(CPU_STK_SIZE) APP_TASK_TOUCH_STK_SIZE / 10,
				(CPU_STK_SIZE) APP_TASK_TOUCH_STK_SIZE,
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
				(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);
			 
	if(err==OS_ERR_NONE) {
//			BSP_UART_Printf(BSP_UART_ID_1, "AppTaskTouch OK");
	}
	

	//�����ӽ��� SENSOR�ӽ���

	OSTaskCreate((OS_TCB     *)&AppTaskSensorTCB,              /* Create the Sensor task                                 */
				(CPU_CHAR   *)"App Task SENSOR",
				(OS_TASK_PTR ) AppTaskSensor,
				(void       *) 0,
				(OS_PRIO     ) APP_TASK_SENSOR_PRIO,
				(CPU_STK    *)&AppTaskSensorStk[0],
				(CPU_STK_SIZE) APP_TASK_SENSOR_STK_SIZE / 10,
				(CPU_STK_SIZE) APP_TASK_SENSOR_STK_SIZE,
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
				(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);
			 
	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppTaskSensor OK");
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
	
	
	 //�����������ӽ���
	
    OSTaskCreate((OS_TCB     *)&AppTaskBuzzerTCB,              /* Create the LED task                                 */
	  		    (CPU_CHAR   *)"App Task Buzzer",
			    (OS_TASK_PTR ) AppTaskBuzzer,
			    (void       *) 0,
			    (OS_PRIO     ) APP_TASK_BUZZER_PRIO,
			    (CPU_STK    *)&AppTaskBuzzerStk[0],
			    (CPU_STK_SIZE) APP_TASK_BUZZER_STK_SIZE / 10,
			    (CPU_STK_SIZE) APP_TASK_BUZZER_STK_SIZE,
			    (OS_MSG_QTY  ) 0u,
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
	
	//�������������ݲɼ�����
	
	OSTaskCreate((OS_TCB     *)&AppTaskCollectionTCB,                                        
				(CPU_CHAR   *)"Collection", 									                     
				(OS_TASK_PTR ) AppTaskCollectin,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_COLLECTION_PRIO,					
				(CPU_STK    *)&AppTaskCollectionStk[0],						
				(CPU_STK_SIZE) APP_TASK_COLLECTION_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_COLLECTION_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppWifi OK");
    }
	
	
	//�������صƴ��Ĵ�����������
	
	OSTaskCreate((OS_TCB     *)&AppTaskHandleTCB,                                        
				(CPU_CHAR   *)"Handle", 									                     
				(OS_TASK_PTR ) AppTaskHandle,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_HANDLE_PRIO,					
				(CPU_STK    *)&AppTaskHandleStk[0],						
				(CPU_STK_SIZE) APP_TASK_HANDLE_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_HANDLE_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppWifi OK");
    }
	
	
	
	//����������ֵ�����
	
	OSTaskCreate((OS_TCB     *)&AppTaskFireTCB,                                        
				(CPU_CHAR   *)"Fire", 									                     
				(OS_TASK_PTR ) AppTaskFire,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_FIRE_PRIO,					
				(CPU_STK    *)&AppTaskFireStk[0],						
				(CPU_STK_SIZE) APP_TASK_FIRE_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_FIRE_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppWifi OK");
    }
	
		
	//���������������
	
	OSTaskCreate((OS_TCB     *)&AppTaskMotorTCB,                                        
				(CPU_CHAR   *)"Motor", 									                     
				(OS_TASK_PTR ) AppTaskMotor,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_MOTOR_PRIO,					
				(CPU_STK    *)&AppTaskMotorStk[0],						
				(CPU_STK_SIZE) APP_TASK_MOTOR_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_MOTOR_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "AppTaskMotor OK");
    }
	
	return DEF_OK;
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
	uint8_t  ssid[31], pwd[31];
	uint8_t  wifi_status = 0x00;
	uint8_t  dat_len = 0;
	uint32_t temp_id;
	OS_ERR   err;
	Local_status request;                     // ����APP�˷��͵�����
	
	(void)p_arg;
	
	// ����Ϊ����ģʽ
	
	BSP_ESP8266_Client_Init();
	
	dat_len = wifi_getSSID(user_info, ssid);   // ��ȡ�ȵ�����
	
	if (0 == dat_len) {                         // ��������,��Ϊ��������������̷߳����ź���
		OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, NULL, &err);
	}
	
	wifi_getPWD(user_info, pwd);               // ��ȡ�ȵ�����
	
	// ��ʼ����
	wifi_status = BSP_ESP8266_WIFI_connect((char*)ssid, (char*)pwd);
	
	while (0x00 == wifi_status) {
		wifi_status = BSP_ESP8266_WIFI_connect((char*)ssid, (char*)pwd);
		
		OSTimeDlyHMSM( 0, 0, 4, 0,              // �߳�����һ��ʱ��
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
	
	// ���ӵ�������
	server_flag = BSP_ESP8266_connect_server();
	
	while (0x00 == server_flag) {
		server_flag = BSP_ESP8266_connect_server();
		
		OSTimeDlyHMSM( 0, 0, 6, 0,              // �߳�����һ��ʱ��
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
	
	// �������ӵ��������ɹ�
	
	while(DEF_ON) {
		dat_len = BSP_ESP8266_Client_Read((uint8_t*)(&request), ALIVE_TIME_OUT);    // ��ȡAPP�˵�����
		
		// ��ʾ��ʵ���ܵ�����������
		
		if (sizeof(request) == dat_len) {                
			
			if (0x0F == request.operation) {                                       // ��ʾAPPҪ����Ӳ���˵�����
			
				// ���ñ���״̬ʱ�������
				
				OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // ռ���ź���
				local_status.window = request.window;
				local_status.led_auto = request.led_auto;
				local_status.led_switch = request.led_switch;
				local_status.window_auto = request.window_auto;
				local_status.power_switch = request.power_switch;
				local_status.warning_flag = request.warning_flag;
				OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // �ͷ��ź���
			
			} else if (0x00 == request.operation) {                               // ��ʾAPPҪ��������
			
				// �ش���������
				
				OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // ռ���ź���
				request.window = local_status.window;
				request.led_auto = local_status.led_auto;
				request.led_switch = local_status.led_switch;
				request.window_auto = local_status.window_auto;
				request.current     = local_status.current;
				request.rain_status = local_status.rain_status;
				request.temperature = local_status.temperature;
				request.power_switch = local_status.power_switch;
				request.warning_flag = local_status.warning_flag;
				OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // �ͷ��ź���
				
				// ���ݽ��ܵ������ݷ�תID
				
				temp_id = request.device_id;
				request.device_id = request.user_id;
				request.user_id = temp_id;
				
				// ����request��������
				
				BSP_ESP8266_Client_Write((uint8_t*)(&request), sizeof(request));
			}			
			
		} else if (0 == dat_len) {                                              // ��ʾ��ʱ����
			
			// ���ʹ���ź�
			
			request.user_id = DEVICE_ID;
			request.device_id = 0x00;                                           // ��ʾΪ�����ź�
			
			BSP_ESP8266_Client_Write((uint8_t*)(&request), sizeof(request));   
		}

	}
	
}





/*
*********************************************************************************************************
*                                          SENSOR TASK
*
* Description : ���������ȡ�������������ݣ����뱣֤���������ȼ���ߣ�ֻ���������ܱ�֤�����ܶ�����������ʱ��
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskSensor (void *p_arg)
{
	OS_ERR     err;

	(void)p_arg;	

	while (DEF_ON) {
					
		OSTimeDlyHMSM( 0, 0, 1, 0,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );

		if(DEF_OK == BSP_18B20_GetTemp()) {
			//BSP_UART_Printf(BSP_UART_ID_1, "Temp: %.4f\n", BSP_18B20_TempTran(temp));
		}
	}
}





/*
*********************************************************************************************************
*                                          Touch TASK
*
* Description : ��������������
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/
static  void  AppTaskTouch (void *p_arg)
{
	OS_ERR  err;
	
	(void)p_arg;

	while(DEF_ON) {
		OSTimeDlyHMSM( 0, 0, 0, 10,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
		
		 GUI_TOUCH_Exec();                     // ��ϸ����������
	}
	
}



/*
*********************************************************************************************************
*                                         LED TASK
*
* Description : ��������ϵͳ��������
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
		
		OSTimeDlyHMSM( 0, 0, 0, 500,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
}




/*
*********************************************************************************************************
*                                          Collectin TASK
*
* Description : �������ɼ����񣬸���ɼ����������������գ�����
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/


static  void  AppTaskCollectin  (void *p_arg)
{
	OS_ERR          err;
	Local_status    temp_state;                     // ���ɼ����������ݴ��ڸñ����У��Լ�С��������
	
	(void) p_arg;
		

	while(DEF_ON) {
		
		// ��������״̬
		
		if (Raining == BSP_Rain_State())
			temp_state.rain_status = 0x0F;
		else 
			temp_state.rain_status = 0x00;
		
		// ������������״̬
		
		if (MQ2_High_Level == BSP_MQ2_State()) 
			temp_state.warning_flag = 0x0F;
		else 
			temp_state.warning_flag = 0x00;
		
		// ���¹���ǿ��״̬
		
		light_state = BSP_Light_State();
		
		// ��ȡ����ǿ��
		
		temp_state.current = BSP_ACS_GetU_Real();
		
		// ��ȡ�¶ȣ��ӻ�������ȡ�����Էǳ�����
		temp_state.temperature = BSP_18B20_TempTran(BSP_18B20_GetTempFast());
		
		
		// ����ʱ�����е����ݿ�����ȫ��״̬����
		
		OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // ռ���ź���
		local_status.current = temp_state.current;
		local_status.rain_status = temp_state.rain_status;
		local_status.temperature = temp_state.temperature;
		
		// ֻ������������������������������������������������ֻ����LCD��APP����
		
		if (0x0F == temp_state.warning_flag)                            
			local_status.warning_flag = temp_state.warning_flag;
		
		OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // �ͷ��ź���
		
		// û���һ��ʱ���ȡһ�δ���������
		
		OSTimeDlyHMSM( 0, 0, 0, 500,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
}



/*
*********************************************************************************************************
*                                          Handle TASK
*
* Description : �������صƣ����ش�����
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskHandle     (void *p_arg)
{
	OS_ERR          err;
	Local_status    temp_state;                     // ��ȡȫ�ֵ�״̬��Ϣ�����ټ�������
	
	(void) p_arg;
		
	/* ��ʾ���� */
	

	while(DEF_ON) {
		
		OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // ռ���ź���
		temp_state.window = local_status.window;
		temp_state.led_auto = local_status.led_auto;
		temp_state.led_switch = local_status.led_switch;
		temp_state.window_auto = local_status.window_auto;
		temp_state.current     = local_status.current;
		temp_state.rain_status = local_status.rain_status;
		temp_state.temperature = local_status.temperature;
		temp_state.power_switch = local_status.power_switch;
		temp_state.warning_flag = local_status.warning_flag;
		OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // �ͷ��ź���
		
		// �����Զ���������
		
		if (0x0F == temp_state.led_switch) {                              // ��������ܿ���Ϊ��
			
			if (0x0F == temp_state.led_auto) {
				if (Light_High_Level == light_state)
					BSP_LIGHT_Off();
				else 
					BSP_LIGHT_On();
					
			} else {                                                      // ����
                BSP_LIGHT_On();                 				
			}
			
		} else {                                                          // �ص�
 			BSP_LIGHT_Off();
		}
		
		// �����Զ��ش�����
		
		if (0x0F == temp_state.window_auto) {
			
			if (0x0F == temp_state.rain_status) {
				OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // ռ���ź���
				local_status.window = 0x00;
				OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // �ͷ��ź���
			}
			
		}
		
		// �����Դ�ܿ��ز���
		
		if (0x0F == temp_state.power_switch) {
			BSP_POWER_SWITCH_On();
			
		} else {
			BSP_POWER_SWITCH_Off();
		}
		
		// ������ֱ������������������ֱ���������ֱ����������ź���
		
		if (0x0F == temp_state.warning_flag && 0x00 == fire_flag) {
			fire_flag = 0x01;                                                    // �ǳ���Ҫ����ֹ�����������ظ������ź��� 
			OSTaskSemPost(&AppTaskFireTCB, OS_OPT_POST_NONE, &err);              // �ᷢ���������
		}
		
		
		OSTimeDlyHMSM( 0, 0, 0, 100,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
}



/*
*********************************************************************************************************
*                                          Handle TASK
*
* Description : �������صƣ����ش�����
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskFire       (void *p_arg)
{
	OS_ERR        err;
	uint8_t       phone[16];
	uint8_t       p_lenth, index, warning;
	
	(void) p_arg;
		
	/* ��ʾ���� */
	

	while(DEF_ON) {
		
		// �����ȴ��ڽ��ź���
		
		OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, NULL, &err);
		
		// ��ʼ������ֱ���
		
		// ���������
		OSTaskSemPost(&AppTaskBuzzerTCB, OS_OPT_POST_NONE, &err);         // �ᷢ���������
		
		// ������Ӧ�û��ж���������û��Ӳ������
		OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // ռ���ź���
		local_status.window = 100;                                           
		OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // �ͷ��ź���
		
		// ���Ͷ���
//		for (index = 0; index < 32; index++) {
//			p_lenth = phone_get(user_info, phone, index);
//			
//			if (p_lenth > 0) {
//				if (REG_NORMAL == BSP_GA6_GetRegSt()) {                       // ģ��ע��ɹ�
//					BSP_GA6_SenTextMSG((char*)phone, "Fire Alarm");
//				}
//			}
//		}

		BSP_UART_Printf(BSP_UART_ID_1, "���Ͷ���\r\n");
		
		// ��֤��ȡ���������Լ��������ᱻ�ظ�����
		
		do {
			OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // ռ���ź���
			warning = local_status.warning_flag;
			OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // �ͷ��ź���
			
			OSTimeDlyHMSM( 0, 0, 0, 10,
						   OS_OPT_TIME_HMSM_STRICT,
                           &err );
			
		} while (0x0F == warning);
		
		fire_flag = 0x00;                                                     // �����ٴα�����
		
	}
	
}





/*
*********************************************************************************************************
*                                          Motor TASK
*
* Description : �����������
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskMotor  (void *p_arg)
{
	OS_ERR      err;
	uint8_t     window_current = 0;           // ��ǰ�����ĽǶ�
	uint8_t     window_target = 0;            // Ŀ�괰���Ƕ�
	
	(void) p_arg;
		
	/* ��ʾ���� */

	while(DEF_ON) {
		
		OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // ռ���ź���
		window_target = local_status.window;                              // ȡ��Ŀ�꿪��
		OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // �ͷ��ź���
		
		if (window_target != window_current) {
			if (window_target > window_current) {
				BSP_Turn_Motor(5, MOTOR_DIR_RIGHT);                       // ���Զ����� 
				window_current++;
			} else {
				BSP_Turn_Motor(5, MOTOR_DIR_LIFT);                        // ���Զ�����
				window_current--;
			}
		} else {
			OSTimeDlyHMSM( 0, 0, 0, 6,                                    // ����һ��ʱ��
						   OS_OPT_TIME_HMSM_STRICT,
						   &err );
		}
	}
	
}




/*
*********************************************************************************************************
*                                          BUZZER TASK
*
* Description : ������ծ�ڷ��ͻ���ʱڤ�������
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskBuzzer (void *p_arg)
{
	OS_ERR     err;
	uint8_t    buzzer_flag;

	(void)p_arg;	

	while (DEF_ON) {
		
		// �ȴ����ֱ����������ź���
		
		OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, NULL, &err);
				
		do {
			OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err);   // ռ���ź���
			buzzer_flag = local_status.warning_flag;
			OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err);                   // �ͷ��ź���
			
			BSP_BUZZER_Func();
		} while (0x0F == buzzer_flag);
		
		// �رշ�����
	}
}






/******************************************************************************************************************************************************
*																		LCD��ͼ�ν�����ش���
*******************************************************************************************************************************************************/


/*********************************************************************
*                          Defines
**********************************************************************/

#define ID_FRAMEWIN_0 (GUI_ID_USER + 0x00)
#define ID_CHECKBOX_0 (GUI_ID_USER + 0x01)
#define ID_CHECKBOX_1 (GUI_ID_USER + 0x02)
#define ID_CHECKBOX_2 (GUI_ID_USER + 0x03)
#define ID_CHECKBOX_3 (GUI_ID_USER + 0x04)
#define ID_BUTTON_0 (GUI_ID_USER + 0x05)
#define ID_TEXT_0 (GUI_ID_USER + 0x06)
#define ID_TEXT_1 (GUI_ID_USER + 0x07)
#define ID_TEXT_2 (GUI_ID_USER + 0x08)
#define ID_TEXT_3 (GUI_ID_USER + 0x09)
#define ID_SLIDER_0 (GUI_ID_USER + 0x0A)



/*********************************************************************
*       _aDialogCreate
**********************************************************************/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Framewin", ID_FRAMEWIN_0, 0, 0, 320, 240, 0, 0x0, 0 },
  { CHECKBOX_CreateIndirect, "Checkbox", ID_CHECKBOX_0, 227, 59, 80, 20, 0, 0x0, 0 },
  { CHECKBOX_CreateIndirect, "Checkbox", ID_CHECKBOX_1, 227, 98, 80, 20, 0, 0x0, 0 },
  { CHECKBOX_CreateIndirect, "Checkbox", ID_CHECKBOX_2, 23, 98, 80, 20, 0, 0x0, 0 },
  { CHECKBOX_CreateIndirect, "Checkbox", ID_CHECKBOX_3, 23, 59, 80, 20, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button", ID_BUTTON_0, 96, 93, 112, 33, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Window Control", ID_TEXT_0, 115, 148, 123, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Temp", ID_TEXT_1, 23, 18, 128, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "CUR", ID_TEXT_2, 225, 18, 85, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Rain", ID_TEXT_3, 133, 49, 80, 20, 0, 0x0, 0 },
  { SLIDER_CreateIndirect, "Slider", ID_SLIDER_0, 50, 178, 220, 20, 0, 0x0, 0 },
};



// ���Դ���
static  char textBuff[20];
static  Local_status  temp_state_UI;
static OS_ERR  err_UI;




/*********************************************************************
*       �ص�����
*********************************************************************/

static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;                      // ���ھ��
  int     NCode;
  int     Id;
  uint8_t val_s = 0;
  static  uint8_t button_state = 0;   // ��ʾ����״̬
  static  uint8_t light_state = 0;
  static  uint8_t light_auto_state = 0;
  static  uint8_t window_auto_state = 0;
  static  uint8_t power_state = 0;

  switch (pMsg->MsgId) {
    // ��ʼ�Ի���
    case WM_INIT_DIALOG:
        // ��ʼ����Դ���ƿ���
        hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_0);
        CHECKBOX_SetText(hItem, "Power");
        CHECKBOX_SetTextColor(hItem, GUI_MAKE_COLOR(0x00008000));

        // ��ʼ�������Զ��ش�����
        hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_1);
        CHECKBOX_SetText(hItem, "W_Auto");
        CHECKBOX_SetTextColor(hItem, GUI_MAKE_COLOR(0x00008000));

        // ��ʼ���Զ��صƿ���
        hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_2);
        CHECKBOX_SetText(hItem, "L_Auto");
        CHECKBOX_SetTextColor(hItem, GUI_MAKE_COLOR(0x00008000));

        // ��ʼ����������
        hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_3);
        CHECKBOX_SetText(hItem, "Light");
        CHECKBOX_SetTextColor(hItem, GUI_MAKE_COLOR(0x00008000));

        // ��ʼ��һ����������
        hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
        BUTTON_SetText(hItem, "FIRE  PRESS");
        BUTTON_SetFont(hItem, GUI_FONT_13HB_1);
        BUTTON_SetTextColor(hItem, BUTTON_CI_PRESSED, 0x6543FE);      // ���ð���ʱ��������ɫ
        BUTTON_SetTextColor(hItem, BUTTON_CI_UNPRESSED, 0x0000FF);    // ����Ϊ����ʱ��������ɫ

        // ��ʼ�����ڿ��������ı�
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
        TEXT_SetFont(hItem, GUI_FONT_13B_1);
        TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x00808040));

        // ��ʼ���¶���ʾ�ı�
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
        TEXT_SetFont(hItem, GUI_FONT_16B_1);
        TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x008B377A));

        // ��ʼ��������ʾ�ı�
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
        TEXT_SetFont(hItem, GUI_FONT_16B_1);
        TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x008B377A));

        // ��ʼ������״̬
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_3);
        TEXT_SetFont(hItem, GUI_FONT_16B_1);
        TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x008B377A));

         // ��ʼ���϶��������÷�Χ�͵�ǰֵ
        hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
        SLIDER_SetRange(hItem, 0, 100);
        SLIDER_SetValue(hItem, 50);
        break;    // WM_INIT_DIALOG:

    // ���ñ�����ɫ
    case WM_PAINT:
        GUI_SetBkColor(0x00D7EBFA);
        GUI_Clear();
        break;

    // ��ʱ��������ڸ��ݵ�Ƭ��״̬����ˢ�½���
    case WM_TIMER:
		// ��ȡ����״̬�����俽������ʱ����
		OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // ռ���ź���
		temp_state_UI.window = local_status.window;
		temp_state_UI.led_auto = local_status.led_auto;
		temp_state_UI.led_switch = local_status.led_switch;
		temp_state_UI.window_auto = local_status.window_auto;
		temp_state_UI.current     = local_status.current;
		temp_state_UI.rain_status = local_status.rain_status;
		temp_state_UI.temperature = local_status.temperature;
		temp_state_UI.power_switch = local_status.power_switch;
		temp_state_UI.warning_flag = local_status.warning_flag;
		OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // �ͷ��ź���
	
		// �����¶�,��ʾ��С�����һλ
		sprintf(textBuff, "TEMP: %.1f `C", temp_state_UI.temperature);
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
        TEXT_SetText(hItem, (const char *)textBuff);
	
		// ���µ�������ʾ��С�����һλ
		sprintf(textBuff, "CUR: %.1f A", temp_state_UI.current);
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
        TEXT_SetText(hItem, (const char *)textBuff);
        
		// ��������״��
		if (temp_state_UI.rain_status == 0x0F) 
			sprintf(textBuff, "W: Raining");
		else 
			sprintf(textBuff, "W: No Rain");
		
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_3);
        TEXT_SetText(hItem, (const char *)textBuff);
		
	
		// ������������light_switch
	    hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_3);
		if (0x0F == temp_state_UI.led_switch) {
			CHECKBOX_SetState(hItem, 1);
			light_state = 1;
		}
		else {
			CHECKBOX_SetState(hItem, 0);
			light_state = 0;
		}
		
		// ���������Զ�����light_auto_switch
		hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_2);
		if (0x0F == temp_state_UI.led_auto) {
			CHECKBOX_SetState(hItem, 1);
			light_auto_state = 1;
		}
		else  {
			CHECKBOX_SetState(hItem, 0);
			light_auto_state = 0;
		}
			
		// �����Զ��ش�window_auto_switch
		hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_1);
		if (0x0F == temp_state_UI.window_auto) {
			CHECKBOX_SetState(hItem, 1);
			window_auto_state = 1;
		}
		else {
			CHECKBOX_SetState(hItem, 0);
			window_auto_state = 0;
		}
		
		// ���µ�Դ����power_switch
		hItem = WM_GetDialogItem(pMsg->hWin, ID_CHECKBOX_0);
		if (0x0F == temp_state_UI.power_switch) {
			CHECKBOX_SetState(hItem, 1);
			power_state = 1;
		}
		else {
			CHECKBOX_SetState(hItem, 0);
			power_state = 0;
		}
		
		// ���´�������window,,�˴����²��ᴥ���ص�����
        hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
        SLIDER_SetValue(hItem, temp_state_UI.window);
		
		// ���±���������
		hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
		if (0x0F == temp_state_UI.warning_flag) {
			button_state = 0x01;
            BUTTON_SetPressed(hItem, 1);
			BUTTON_SetText(hItem, "ALARM CANCEL");
		} else {
			button_state = 0x00;
            BUTTON_SetPressed(hItem, 0);
			BUTTON_SetText(hItem, "FIRE  PRESS");
		}
	
		WM_RestartTimer(pMsg->Data.v, 500);             // ��ʼ����ʱ����ֵ500ms
        break;

		
    // ������ͼ������
    case WM_NOTIFY_PARENT:
        Id    = WM_GetId(pMsg->hWinSrc);
        NCode = pMsg->Data.v;

        switch(Id) {
            case ID_CHECKBOX_0:                          // power
                switch(NCode) {
                    case WM_NOTIFICATION_CLICKED:        // �����ʱ����ѡ��

                        break;

                    case WM_NOTIFICATION_RELEASED:       // ����ͷ�ʱ����ȡ��ѡ��
						power_state++;
						power_state &= 0x01;
						OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // ռ���ź���
						if (0 == power_state) {
							local_status.power_switch = 0x00;
						} else {
							local_status.power_switch = 0x0F;
						}
						OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // �ͷ��ź���

                        break;

                    case WM_NOTIFICATION_VALUE_CHANGED:  // ״̬�����ı�ʱ
						
                        break;
                }
                break; // ID_CHECKBOX_0

            case ID_CHECKBOX_1:                          // w_auto
                switch(NCode) {
                    case WM_NOTIFICATION_CLICKED:        // �����ʱ����ѡ��

                        break;

                    case WM_NOTIFICATION_RELEASED:       // ����ͷ�ʱ����ȡ��ѡ��
						window_auto_state++;
						window_auto_state &= 0x01;
						OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // ռ���ź���
						if (0 == window_auto_state) {
							local_status.window_auto = 0x00;
						} else {
							local_status.window_auto = 0x0F;
						}
						OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // �ͷ��ź���
                        break;

                    case WM_NOTIFICATION_VALUE_CHANGED:  // ״̬�����ı�ʱ
						
                        break;

                }
                break; // ID_CHECKBOX_1

            case ID_CHECKBOX_2:                          // l_auto
                switch(NCode) {
                    case WM_NOTIFICATION_CLICKED:        // �����ʱ����ѡ��

                        break;

                    case WM_NOTIFICATION_RELEASED:       // ����ͷ�ʱ����ȡ��ѡ��
						light_auto_state++;
						light_auto_state &= 0x01;
						OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // ռ���ź���
						if (0 == light_auto_state) {
							local_status.led_auto = 0x00;
						} else {
							local_status.led_auto = 0x0F;
						}
						OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // �ͷ��ź���
                        break;

                    case WM_NOTIFICATION_VALUE_CHANGED:  // ״̬�����ı�ʱ
						
                        break;
                }
                break; // ID_CHECKBOX_2

            case ID_CHECKBOX_3:                         // light
                switch(NCode) {
                    case WM_NOTIFICATION_CLICKED:       // �����ʱ����ѡ��

                        break;

                    case WM_NOTIFICATION_RELEASED:      // ����ͷ�ʱ����ȡ��ѡ��
						light_state++;
						light_state &= 0x01;
						OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // ռ���ź���
						if (0 == light_state) {
							local_status.led_switch = 0x00;
						} else {
							local_status.led_switch = 0x0F;
						}
						OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // �ͷ��ź���
                        break;

                    case WM_NOTIFICATION_VALUE_CHANGED: // ״̬�����ı�ʱ
						
                        break;
                }
                break; // ID_CHECKBOX_3

			case ID_BUTTON_0:
				switch(NCode) {
					case WM_NOTIFICATION_CLICKED:   // ����������ʱ

						break;

					case WM_NOTIFICATION_RELEASED:  // �����ͷ�ʱ
						button_state++;
						button_state &= 0x01;
						hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
						BUTTON_SetPressed(hItem, button_state);
						if (0 == button_state) {
							OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // ռ���ź���
							local_status.warning_flag = 0x00;
							OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // �ͷ��ź���
							BUTTON_SetText(hItem, "FIRE  PRESS");
						} else {
							OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // ռ���ź���
							local_status.warning_flag = 0x0F;
							OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // �ͷ��ź���
							BUTTON_SetText(hItem, "ALARM CANCEL");
						}
			
						break;
				}
				break; // ID_BUTTON_0

            case ID_SLIDER_0:
                switch(NCode) {
                    case WM_NOTIFICATION_CLICKED:       // �϶��������ʱ

                        break;

                    case WM_NOTIFICATION_RELEASED:      // �϶������ͷ�ʱ
                        hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
                        val_s = SLIDER_GetValue(hItem);
						OSSemPend(&StatusSemLock, 0, OS_OPT_PEND_BLOCKING, NULL, &err_UI);   // ռ���ź���
						local_status.window = val_s;
						OSSemPost(&StatusSemLock, OS_OPT_POST_1, &err_UI);                   // �ͷ��ź���
                        break;

                    case WM_NOTIFICATION_VALUE_CHANGED:  // �϶������ݷ����ı�ʱ

                        break;

                }
                break; // ID_SLIDER_0
        }
        break; // WM_NOTIFY_PARENT

    default:
        WM_DefaultProc(pMsg);
        break;
  }
}


/*********************************************************************
*       CreateFramewin
**********************************************************************/
WM_HWIN CreateFramewin(void);
WM_HWIN CreateFramewin(void) {
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  return hWin;
}


/*********************************************************************
*       MainTask UI�߳���ں���
**********************************************************************/

void MainTask(void)
{
    WM_HWIN   hDlg;                                 // ���ھ��
	
    // ����ʹ��ʹ���ڴ��豸����ֹ��˸������GUI_Init()֮ǰ���Ա�֤����
    // ���ڶ���ʹ�ö�̬�ڴ��豸
    WM_SetCreateFlags(WM_CF_MEMDEV);

    GUI_Init();
	

    // ����һ�����ڲ��ҷ���һ�����,����Ϊ�䴴��һ����ʱ��
    hDlg = CreateFramewin();

    // Ϊ���ڴ���һ����ʱ������ʱ������ͻᴥ�������ڸ��½����View
    WM_CreateTimer(WM_GetClientWindow(hDlg), 0, 1000, 0);

    while (1) {
        GUI_Delay(10);
    }

}





