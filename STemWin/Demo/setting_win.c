#include "GUI.h"


void SettingMainTask(void)
{
    // ��ʼ emWin
    GUI_Init();

    // ���ñ�����ɫ
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();

    // ��������
    GUI_SetFont(&GUI_Font8x16);

    /* ��������ַ�����ʾ ��ʵ��һ�»��� */
    GUI_DispStringAt("In setting mode...", 2, 2);
    GUI_DispStringAt("Please connect this device...", 2, 18);

    while (1)
    {
        GUI_Delay(10);
    }
}


/*************************** End of file ****************************/
