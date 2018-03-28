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

#include "my_app_cfg.h"
#include  "user_info.h"
#include  "stdlib.h"
#include  "string.h"

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




/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskSensor (void *p_arg);

static  void  AppTaskLed    (void *p_arg);

static  void  AppTaskTouch  (void *p_arg);

static  void  AppTaskMotor  (void *p_arg);

static  void  AppTaskWifi   (void *p_arg);




/*
*********************************************************************************************************
*                                       LOCAL DEFINES
*********************************************************************************************************
*/

#define     DEVICE_ID                     12345         // Ӳ���豸ΨһID
#define     ALIVE_TIME_OUT                2000          // ��λΪʱ�ӽ�������
 

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




/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/




static       OS_SEM               StatusSemLock;          // ���ڶ�ռ����״̬ȫ�ֱ���  

static       Local_status         local_status;           // ȫ��״̬

static       uint8_t              server_flag = 0x00;     // ������������״̬         

static       InfoStruct           *user_info;             // ʹ�ø��ڴ�������Flash�е��û�����




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
				(OS_MSG_QTY  ) 5u,
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
				(OS_MSG_QTY  ) 5u,
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
	
#if 0		
	
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
	

	
	
	
#endif
	
	
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
	
	// �Ƚ�Flash�е����ݶ����ڴ�
	
	get_user_info(user_info, &AppTaskWifiTCB); 
	
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
		
//		BSP_UART_Printf(BSP_UART_ID_1,"CH1: %d\r\n", BSP_ADC_GetDat(BSP_ADC_CH1));
//		BSP_UART_Printf(BSP_UART_ID_1,"CH2: %d\r\n", BSP_ADC_GetDat(BSP_ADC_CH2));
//		BSP_UART_Printf(BSP_UART_ID_1,"CH3: %d\r\n\r\n", BSP_ADC_GetDat(BSP_ADC_CH3));
		
		
//		BSP_UART_Printf(BSP_UART_ID_1, "i: %f\r\n", BSP_ACS_GetS_Real());
		
		BSP_LED_Toggle(3);
		
		OSTimeDlyHMSM( 0, 0, 0, 200,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
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
	
	(void) p_arg;
		
	/* ��ʾ���� */

	while(DEF_ON) {
//		BSP_Turn_Motor(360, MOTOR_DIR_RIGHT);
		
		OSTimeDlyHMSM( 0, 0, 6, 0,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
	
}







