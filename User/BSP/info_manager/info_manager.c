
#include "info_manager.h"
#include <string.h>



/*
*********************************************************************************************************
*                                        phone_add()
*
* Description : 向连续内存空间增加一条电话号码电话号码长度必须指定
*
* Argument(s) : buf是信息结构体， body是真实电话号码， phoneLen是电话号码长度
*
* Return(s)   : 成功返回1，失败返回0
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
	
	if (phoneLen > 11)                                  // 电话号码太长
		return 0;
	
	for (index = 0; index < 32; index++) {
		if(0 == *(*(ptr+index))) {                      // 表示无电话号码
			memcpy(*(ptr+index)+1, body, phoneLen);
			*(*(ptr+index)) = phoneLen;                 // 更新长度
			return 1;
		}
	}
	
	return 0;
}


/*
*********************************************************************************************************
*                                        phone_clean()
*
* Description : 清空内存中所有电话记录
*
* Argument(s) : buf是信息结构体
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
		*(*(ptr+index)) = 0;                         // 长度为复制为0
	}	
}	



/*
*********************************************************************************************************
*                                        phone_del()
*
* Description : 删除指定索引的电话号码
*
* Argument(s) : buf是信息结构体, index是索引
*
* Return(s)   : 成功返回1，失败返回0
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
	
	*(*(ptr+index)) = 0;                            // 直接清空电话号码长度
}


/*
*********************************************************************************************************
*                                        phone_get()
*
* Description : 获取指定位置的电话号码
*
* Argument(s) : buf是信息结构体, body为返回的电话体,index是索引
*
* Return(s)   : 成功返回电话号码长度，失败返回0
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
* Description : 获取wifiSSID
*
* Argument(s) : buf是信息结构体, ssid为wifi名
*
* Return(s)   : 如果存在则返回wifi名长度否则返回0
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
* Description : 获取wifiPWD
*
* Argument(s) : buf是信息结构体, pwd为wifi密码
*
* Return(s)   : 成功者返回wifi密码长度，否则返回0
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
* Description : 设置wifiPWD
*
* Argument(s) : buf是信息结构体, pwd为wifi密码, len为密码长度
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
* Description : 设置wifiSSID
*
* Argument(s) : buf是信息结构体, ssid为wifi名字, len为名字长度
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






