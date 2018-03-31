#include "GUI.h"


void SettingMainTask(void)
{
    // 初始 emWin
    GUI_Init();

    // 设置背景颜色
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();

    // 设置字体
    GUI_SetFont(&GUI_Font8x16);

    /* 这里调用字符串显示 并实现一下换行 */
    GUI_DispStringAt("In setting mode...", 2, 2);
    GUI_DispStringAt("Please connect this device...", 2, 18);

    while (1)
    {
        GUI_Delay(10);
    }
}


/*************************** End of file ****************************/
