/*******************************************************************************
* @file      : Display.h
* @author    : Jiangsh
* @version   : V1.0.0
* @date      : Sat Apr 21 2018
* @brief     : 
********************************************************************************
* @attention :如果本文本中存在汉字变量，文本要转化成GB2312编码格式保存和打开
*
*
*/
/* 条件编译-------------------------------------------------------------------*/
#ifndef _DISPLAY_H
#define _DISPLAY_H



/*头文件----------------------------------------------------------------------*/
///添加头文件
#include "LCDConfig.h"

/*宏定义----------------------------------------------------------------------*/
///添加宏定义
typedef enum
{
    CombinedActivePowerEnergy,          //组合有功电量
	PositiveActivePowerEnergy,	        //正向有功电量
	ReverseActivePowerEnergy,		    //反向有功电量
    CombinedOneReactivePowerEnergy,     //组合1无功电量
    CombinedTwoReactivePowerEnergy,     //组合2无功电量
	PositiveReactivePowerEnergy,	    //正向无功电量
	ReverseReactivePowerEnergy,		    //反向无功电量
    FirstQuadrantReactivePowerEnergy,   //第1象限无功电量
    SecondQuadrantReactivePowerEnergy,  //第2象限无功电量
    ThirdQuadrantReactivePowerEnergy,   //第3象限无功电量
    FourthQuadrantReactivePowerEnergy,  //第4象限无功电量
    PositiveApparentEnergy,             //正向视在电量
    ReverseApparentEnergy               //反向视在电量
}ENERGY_TYPE;                           //电量种类

typedef enum
{
    TotalPhase,                         //合相
    APhase,                             //A相
    BPhase,                             //B相
    CPhase                              //C相
}PHASE_TYPE;                            //相位种类


typedef enum
{
    Plus,                      //正
    Minus                      //负
}PLUS_MINUS;                   //正负标志


typedef enum
{
    DisplayHighZero,           //显示高位零
    NoDisplayHighZero          //不显示高位零
}HIGHZERO_TYPE;                //高位显零标志

typedef enum
{
    NoDisplaySplitScreen,               //不显示分屏
    DisplaySplitScreenZero,             //显示分屏序号00
    DisplaySplitScreenOne,              //显示分屏序号01
    DisplaySplitScreenTwo,              //显示分屏序号02
    DisplaySplitScreenThree             //显示分屏序号03
    //可以继续增加...
}SPLITSCREENDISPLAY_TYPE;               //分屏显示标志

/*声明外部函数----------------------------------------------------------------*/
/** 
 * @brief  显示电量函数
 * @note  
 * @param  phase: 代表相位 具体参见PHASE_TYPE枚举 
 * @param  engerytype: 电量种类，具体参见ENERGY_TYPE枚举
 * @param  date:    日期，暂时支持0~12，0表示当前  其他代表上x月
 * @param  rate:    费率 0~12，其中0代表总，其他代表Tx
 * @param  engerypoint: 指向电量存储的数组，默认6字节BCD码，最低字节代表第3,4小数
 * @param  decimalpoint: 代表显示的电量显示几位小数，0~4
 * @param  plusminus: 代表是否显示负号，    Plus代表不显示，Minus代表显示
 * @param  displayhighzero: 高位是否显零，具体参见HIGHZERO_TYPE枚举
 * @retval None
 */
extern void Display_Engery(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate,unsigned char* engerypoint,unsigned char decimalpoint,PLUS_MINUS plusminus,HIGHZERO_TYPE displayhighzero);

/** 
 * @brief  显示当前日期
 * @note   
 * @param  datepoint: 指向电量存储的数组，默认3字节BCD码，最低字节代表日
 * @retval None
 */
extern void Display_CurrentDate(unsigned char* datepoint);

/** 
 * @brief  显示当前时间
 * @note   
 * @param  timepoint: 指向时间存储的数组，默认3字节BCD码，最低字节代表秒
 * @retval None
 */
extern void Display_CurrentTime(unsigned char* timepoint);

/** 
 * @brief  显示当前剩余金额
 * @note   三相显示“当前剩余电费” 单相显示“当前剩余金额”
 * @param  amountpoint: 指向剩余金额存储的数组，默认4字节BCD码，最低字节代表小数1、2位
 * @param  displayhighzero: 高位是否显零，0代表不显示， 1代表显示
 * @retval None
 */
extern void Display_RemainingAmount(unsigned char* amountpoint,unsigned char displayhighzero);

/** 
 * @brief  显示当前透支金额
 * @note   三相显示“当前透支电费” 单相显示“当前透支金额”
 * @param  amountpoint: 指向透支金额存储的数组，默认4字节BCD码，最低字节代表小数1、2位
 * @param  displayhighzero: 高位是否显零，0代表不显示， 1代表显示
 * @retval None
 */
extern void Display_OverdraftAmount(unsigned char* amountpoint,unsigned char displayhighzero);


#endif
/*end------------------------------------------------------------------------*/
