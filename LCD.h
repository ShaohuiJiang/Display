/*******************************************************************************
* @file      : LCD.h
* @author    : Jiangsh
* @version   : V1.0.0
* @date      : Thu Apr 12 2018
* @brief     : 
********************************************************************************
* @attention :如果本文本中存在汉字变量，文本要转化成GB2312编码格式保存和打开
*
*
*/
/* 条件编译-------------------------------------------------------------------*/
#ifndef _LCD_H
#define _LCD_H

/*头文件---------------------------------------------------------------------*/
#include "LCDConfig.h"

/*宏定义---------------------------------------------------------------------*/
/* LCD的点阵数 */        
#define com   80        //COM数
#define seg   160       //段数
#define Page  10        //COM组成的页数

#if (MeterType == ThreePhaseMeter)      //三相模式

//注意：象限区和中文提示区在com上都是重叠的，只是seg范围不一样
/* 象限区的COM地址范围（前闭后开）12p */
#define QuadrantAreaStartCom 0
#define QuadrantAreaEndCom   12

/* 象限区的页地址范围 （前闭后开）2byte*/
#define QuadrantAreaStartPageCom     0
#define QuadrantAreaAreaEndPageCom   2

/* 象限区的SEG地址范围 （前闭后开）12p*/
#define QuadrantAreaStartSeg 0
#define QuadrantAreaEndSeg   12

#endif

#if (MeterType == ThreePhaseMeter)
/* 中文提示区的COM地址范围（前闭后开） 12p*/
#define ChineseHintAreaStartCom 0
#define ChineseHintAreaEndCom   12

/* 中文提示区的页地址范围 （前闭后开）2bytes*/
#define ChineseHintAreaStartPageCom 0
#define ChineseHintAreaEndPageCom   2

/* 中文提示区的Seg地址范围（前闭后开） 145p*/
#define ChineseHintAreaStartSeg 15
#define ChineseHintAreaEndSeg   seg

#else      

/* 中文提示区的COM地址范围（前闭后开） 14p*/
#define ChineseHintAreaStartCom 0
#define ChineseHintAreaEndCom   14

/* 中文提示区的页地址范围 （前闭后开）2bytes*/
#define ChineseHintAreaStartPageCom 0
#define ChineseHintAreaEndPageCom   2

/* 中文提示区的Seg地址范围（前闭后开） 160p*/
#define ChineseHintAreaStartSeg 0
#define ChineseHintAreaEndSeg   160

#endif


#if (MeterType == ThreePhaseMeter)       //三相模式

/* 中文提示区的分屏的COM地址范围（前闭后开） 12p*/
#define SplitWindowAreaStartCom 0
#define SplitWindowAreaEndCom   12

/* 中文提示区的分屏的页地址范围 （前闭后开）2bytes*/
#define SplitWindowAreaStartPageCom 0
#define SplitWindowAreaEndPageCom   2

/* 中文提示区的分屏seg地址范围（前闭后开）13p */
#define SplitWindowAreaEndseg   seg
#define SplitWindowAreaStartseg (SplitWindowAreaEndseg-13)

#else

/* 中文提示区的COM地址范围（前闭后开） 14p*/
#define SplitWindowAreaStartCom 0
#define SplitWindowAreaEndCom   14

/* 中文提示区的页地址范围 （前闭后开）2bytes*/
#define SplitWindowAreaStartPageCom 0
#define SplitWindowAreaEndPageCom   2

/* 中文提示区的分屏seg地址范围（前闭后开） */
#define SplitWindowAreaEndseg  seg
#define SplitWindowAreaStartseg (SplitWindowAreaEndseg-14)
#endif



#if (MeterType == ThreePhaseMeter)       //三相模式

/* 数字区的COM地址范围 （前闭后开）36p*/
#define NumberAreaStartCom      15
#define NumberAreaEndCom        51

/* 数字区的页地址范围 （前闭后开）6bytes*/
#define NumberAreaStartPageCom   1
#define NumberAreaEndPageCom     7  

#else

/* 数字区的COM地址范围 （前闭后开）36p*/
#define NumberAreaStartCom      16
#define NumberAreaEndCom        52

/* 数字区的页地址范围 （前闭后开）5bytes*/
#define NumberAreaStartPageCom   2
#define NumberAreaEndPageCom     7  

#endif

/* 数字区的Seg地址范围 （前闭后开）160p*/
#define NumberAreaStartSeg      0
#define NumberAreaEndSeg        seg

  

/* 中文报警的COM地址范围（前闭后开） 12p */
#define AlarmHintAreaStartCom     54
#define AlarmHintAreaEndCom       66

/* 中文报警区的页地址范围（前闭后开）3Byts */
#define AlarmHintAreaStartPageCom  6
#define AlarmHintAreaEndPageCom    9

/* 中文报警区的Seg地址范围（前闭后开） 135p */
#define AlarmHintAreaStartSeg     0
#define AlarmHintAreaEndSeg      135


/* 单位区的COM地址范围（前闭后开） 12p */
#define UnitAreaStartCom     54
#define UnitAreaEndCom       66

/* 单位区的页地址范围（前闭后开）3Byts */
#define UnitAreaStartPageCom  6
#define UnitAreaEndPageCom    9

/* 单位区的Seg地址范围（前闭后开） 25p */
#define UnitAreaStartSeg     135
#define UnitAreaEndSeg       seg



#if (MeterType == ThreePhaseMeter)       //三相模式

/* 字符状态提示区的COM地址范围（前闭后开） 11p */
#define StatusHintAreaStartCom       69
#define StatusHintAreaEndCom         80

#else

/* 字符状态提示区的COM地址范围（前闭后开） 11p */
#define StatusHintAreaStartCom       68
#define StatusHintAreaEndCom         79

#endif

/* 字符状态提示区区的页地址范围（前闭后开）2bytes */
#define StatusHintAreaStartPageCom    8
#define StatusHintAreaEndPageCom      10

/* 字符状态提示区的Seg地址范围（前闭后开）  */
#define StatusHintAreaStartSeg        0
#define StatusHintAreaEndSeg          seg

#define display                        1
#define clear                          0




/*声明外部函数----------------------------------------------------------------*/

/* LCD的配置初始化函数，确保调用该函数前，LCD_GPIO_Init已经被执行过 */
extern void LCD_Config_Init(void);

/* 设置LCD的ID，不知道用来干吗？ */
extern void Set_OTP_Register(void);

/* LCD的总初始化函数，集合了管脚初始化、配置初始化和设置ID； */
extern void LCD_Init(void);

/* 关闭点阵液晶，同时复位脚置低 */
extern void Closed_LCD(void);

/* 往LCDRAM_BUf的指定位置显示或者清除一个字符(包括汉字、字母、数字、符号) */
//extern void InputCharacter_to_LCDRAM_Buf(unsigned char x,unsigned char y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear);
extern void InputCharacter_to_LCDRAM_Buf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear);

/* LCDRAM_Buf数组清零 */
extern void Clear_LCDRAM_Buf(void);

#if (MeterType == ThreePhaseMeter)
/* 清除LCDRAM_Buf数组的象限区的内容 */
extern void Clear_QuadrantArea_Of_LCDRAM_Buf(void);
#endif 

/* 清除LCDRAM_Buf数组中指定范围内的数据 */
//extern void Clear_PartArea_of_LCDRAM_Buf(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2);
extern void Clear_PartArea_of_LCDRAM_Buf(unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2);
/* 清除LCDRAM_Buf数组的中文提示区的内容 */
extern void Clear_ChineseHintArea_Of_LCDRAM_Buf(void);

/* 清除LCDRAM_Buf数组的数字区的内容 */
extern void Clear_NumberArea_Of_LCDRAM_Buf(void);

/* 清除LCDRAM_Buf数组的报警提示区的内容 */
extern void Clear_AlarmHintArea_Of_LCDRAM_Buf(void);

/* 清除LCDRAM_Buf数组的单位区的内容 */
extern void Clear_UnitArea_Of_LCDRAM_Buf(void);

/* 清除LCDRAM_Buf数组的状态提示区的内容 */
extern void Clear_StatusHintArea_Of_LCDRAM_Buf(void);

/* 刷新LCD整屏的显示内容 */
extern void Refresh_LCD_DDRAM(void);

#if (MeterType == ThreePhaseMeter)
/* 刷新LCD的象限区显示内容 */
extern void Refresh_QuadrantArea_of_LCD_DDRAM(void);
#endif 

/* 刷新LCD的中文提示区显示内容 */
extern void Refresh_ChineseHintArea_of_LCD_DDRAM(void);

/* 刷新LCD的数字区显示内容 */
extern void Refresh_NumberArea_of_LCD_DDRAM(void);

/* 刷新LCD的报警区显示内容 */
extern void Refresh_AlarmHintArea_of_LCD_DDRAM(void);

/* 刷新LCD的单位区显示内容 */
extern void Refresh_UnitArea_of_LCD_DDRAM(void);
/* 刷新LCD的状态区显示内容 */
extern void Refresh_StatusHintArea_of_LCD_DDRAM(void);

extern void Wirte_LCDRAM_Buf(unsigned short x, unsigned short y, unsigned char Buf);

#endif
/*end------------------------------------------------------------------------*/
