
#include "info_manager.h"
#include <string.h>



/*
*********************************************************************************************************
*                                        phone_add()
*
* Description : �������ڴ�ռ�����һ���绰����绰���볤�ȱ���ָ��
*
* Argument(s) : buf����Ϣ�ṹ�壬 body����ʵ�绰���룬 phoneLen�ǵ绰���볤��
*
* Return(s)   : �ɹ�����1��ʧ�ܷ���0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t phone_add(InfoStruct *buf, uint8_t *body, uint8_t phoneLen) 
{
	uint8_t index;
	uint8_t (*ptr)[12] = buf->phone;
	
	if (phoneLen > 11)                                  // �绰����̫��
		return 0;
	
	for (index = 0; index < 32; index++) {
		if(0 == *(*(ptr+index))) {                      // ��ʾ�޵绰����
			memcpy(*(ptr+index)+1, body, phoneLen);
			*(*(ptr+index)) = phoneLen;                 // ���³���
			return 1;
		}
	}
	
	return 0;
}


/*
*********************************************************************************************************
*                                        phone_clean()
*
* Description : ����ڴ������е绰��¼
*
* Argument(s) : buf����Ϣ�ṹ��
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/


void phone_clean(InfoStruct *buf)
{
	uint8_t index;
	uint8_t (*ptr)[12] = buf->phone;
	
	for (index = 0; index < 32; index++) {
		*(*(ptr+index)) = 0;                         // ����Ϊ����Ϊ0
	}	
}	



/*
*********************************************************************************************************
*                                        phone_del()
*
* Description : ɾ��ָ�������ĵ绰����
*
* Argument(s) : buf����Ϣ�ṹ��, index������
*
* Return(s)   : �ɹ�����1��ʧ�ܷ���0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void phone_del(InfoStruct *buf, uint8_t index)
{
	uint8_t (*ptr)[12] = buf->phone;
	
	if (index > 31)
		return;
	
	*(*(ptr+index)) = 0;                            // ֱ����յ绰���볤��
}


/*
*********************************************************************************************************
*                                        phone_get()
*
* Description : ��ȡָ��λ�õĵ绰����
*
* Argument(s) : buf����Ϣ�ṹ��, bodyΪ���صĵ绰��,index������
*
* Return(s)   : �ɹ����ص绰���볤�ȣ�ʧ�ܷ���0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t phone_get(InfoStruct *buf, uint8_t *body, uint8_t index)
{
	uint8_t  phone_len;
	uint8_t (*ptr)[12] = buf->phone;
	
	if (index > 31)
        return 0;
	
	phone_len = *(*(ptr+index));
	
	if (0 == phone_len || phone_len > 11)
		return 0;
	
	memcpy(body, (*(ptr+index))+1, phone_len);
	
	return phone_len;
}


/*
*********************************************************************************************************
*                                        wifi_getSSID();
*
* Description : ��ȡwifiSSID
*
* Argument(s) : buf����Ϣ�ṹ��, ssidΪwifi��
*
* Return(s)   : ��������򷵻�wifi�����ȷ��򷵻�0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t wifi_getSSID(InfoStruct *buf, uint8_t *ssid)
{
	uint8_t ssid_len;
	uint8_t *ptr = buf->wifi_ssid;
	
	ssid_len = *ptr;
	
	if (0 == ssid_len || ssid_len > 31)
		return 0;
	
	memcpy(ssid, ptr+1, ssid_len);
	
	return ssid_len;
}


/*
*********************************************************************************************************
*                                        wifi_getPWD();
*
* Description : ��ȡwifiPWD
*
* Argument(s) : buf����Ϣ�ṹ��, pwdΪwifi����
*
* Return(s)   : �ɹ��߷���wifi���볤�ȣ����򷵻�0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t wifi_getPWD(InfoStruct *buf, uint8_t *pwd)
{
	uint8_t pwd_len;
	uint8_t *ptr = buf->wifi_pwd;
	
	pwd_len = *ptr;
	
	if (0 == pwd_len || pwd_len > 31)
		return 0;
	
	memcpy(pwd, ptr+1, pwd_len);
	
	return pwd_len;
}


/*
*********************************************************************************************************
*                                        wifi_setPWD();
*
* Description : ����wifiPWD
*
* Argument(s) : buf����Ϣ�ṹ��, pwdΪwifi����, lenΪ���볤��
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void wifi_setPWD(InfoStruct *buf, uint8_t *pwd, uint8_t len)
{
	uint8_t *ptr = buf->wifi_pwd;
	
	if (len > 31)
		return;
	
	*ptr = len;
	memcpy(ptr+1, pwd, len);
}



/*
*********************************************************************************************************
*                                        wifi_setSSID();
*
* Description : ����wifiSSID
*
* Argument(s) : buf����Ϣ�ṹ��, ssidΪwifi����, lenΪ���ֳ���
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void wifi_setSSID(InfoStruct *buf, uint8_t *ssid, uint8_t len)
{
	uint8_t *ptr = buf->wifi_ssid;
	
	if (len > 31)
		return;
	
	*ptr = len;
	memcpy(ptr+1, ssid, len);
}






