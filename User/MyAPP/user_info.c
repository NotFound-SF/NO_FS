
/*
*********************************************************************************************************
*                          ���ļ��ṩ��flash��ȡ�û���Ϣ�ķ���
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  "user_info.h"


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        save_user_info()
*
* Description : ���ڴ�ṹ�е�����д�뵽flash�У�д�����ŷ���
*
* Argument(s) : dat:��Flash�ж�ȡ�����ݽ�������Dat
*             : task_tcb�����øú���������TCB��ԣ���������������
*
* Return(s)   : none
*
* Caller(s)   : APP
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  save_user_info(InfoStruct *dat, OS_TCB *task_tcb)
{
	uint8_t            buf[USER_INFO_BUF_SIZE];               // ���ڴ��ͷ��Ϣ�����ݵĿռ� 
	uint16_t           index;
	uint8_t           *dest = buf+sizeof(FLASH_DRIVER_MSG);   // ƫ�ƹ�ͷ��Ϣ
	uint8_t           *source = (uint8_t *)dat;
	FLASH_DRIVER_MSG  *request = (FLASH_DRIVER_MSG *)buf;
	
	// ��ʼ�����������ͷ��Ϣ
	request ->option   = FLASH_OPTION_W;                       // д����
	request ->task_tcb = task_tcb;                             // �����ߵ�TCB
	request ->addr     = USER_INFO_BASE;                       // Ҫ��ȡ��flash��ַ
	request ->len      = USER_INFO_SIZE;                       // ��ȡ���ֽڳ���
	
	// �����ʵ��Ϣ��
	for (index = 0; index < sizeof(InfoStruct); index++) {
		*(dest+index) = *(source+index);                       
	}
	
	BSP_FLASH_Drivet_Request(request);                         // ��Flash����������,������ᱻ����
}



/*
*********************************************************************************************************
*                                        get_user_info()
*
* Description : ����flash�е��û����ݶ����ڴ棬��ȡ������ŷ���
*
* Argument(s) : dat:��Flash�ж�ȡ�����ݽ�������Dat
*             : task_tcb�����øú���������TCB��ԣ���������������
*
* Return(s)   : none
*
* Caller(s)   : APP
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  get_user_info(InfoStruct *dat, OS_TCB *task_tcb)
{
	uint16_t           index;
	uint8_t            buf[USER_INFO_BUF_SIZE];                // ���ڴ��ͷ��Ϣ�����ݵĿռ� 
	uint8_t           *dest = (uint8_t *)dat;                  // Ŀ���ַ
	uint8_t           *source = buf+sizeof(FLASH_DRIVER_MSG);  // ƫ�ƹ�ͷ��Ϣ����ԭ��ַ
	FLASH_DRIVER_MSG  *request = (FLASH_DRIVER_MSG *)buf;
	
	// ��ʼ�����������ͷ��Ϣ
	request ->option   = FLASH_OPTION_R;                       // ������
	request ->task_tcb = task_tcb;                             // �����ߵ�TCB
	request ->addr     = USER_INFO_BASE;                       // Ҫ��ȡ��flash��ַ
	request ->len      = USER_INFO_SIZE;                       // ��ȡ���ֽڳ���
	
	BSP_FLASH_Drivet_Request(request);                         // ��Flash����������,������ᱻ����
	
	// �����ʵ��Ϣ��
	for (index = 0; index < sizeof(InfoStruct); index++) {
		*(dest+index) = *(source+index);                       
	}
	
}








