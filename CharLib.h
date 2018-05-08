/*******************************************************************************
* @file      : CharLib.h
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
#ifndef _CHARLIB_H
#define _CHARLIB_H

/*头文件----------------------------------------------------------------------*/
#include "LCDConfig.h"

/*宏定义----------------------------------------------------------------------*/
typedef struct
{
        unsigned short CharGbk;             //GBK码     
  const unsigned char* CharAddress;         //对应的GBK码的字符所在的字库位置

}GBK_CHARADDRESS_STRUCT;                    //字符的GBK码字库数组起始地址结构体


/* 显示字符尺寸 seg*100+com */

#if (MeterType == ThreePhaseMeter)

#define Size_6P12P       612    //中文提示区数字字母
#define Size_13P12P     1312    //中文提示区分屏字符
#else
#define Size_7P14P       714    //中文提示区数字字母
#define Size_14P14P     1414    //中文提示区汉字
#endif 

#define Size_12P12P     1212    //三相的中文提示区汉字、报警提示区汉字、单位区汉字和三相的象限区象限标志

#define Size_18P36P   1836      //数字区数字
#define Size_4P36P     436      //数字区小数点
#define Size_8P36P     836      //数字区负号

#define Size_26P12P   2612      //报警提示区单位
#define Size_10P11P   1011      //状态提示区标志

#define Size_8P12P     812      //单位区字符
#define Size_24P12P   2412      //单位区字符
#define Size_25P12P   2512      //单位区字符

#define Size_6P11P     611      //状态提示区字符
#define Size_7P11P     711      //状态提示区字符
#define Size_8P11P     811      //状态提示区字符
#define Size_9P11P     911      //状态提示区字符
#define Size_11P11P    1111     //状态提示区字符
#define Size_12P11P    1211     //状态提示区字符
#define Size_13P11P    1311     //状态提示区字符
#define Size_14P11P    1411     //状态提示区字符


/* 各个字模数字的大小 */
#if (MeterType == ThreePhaseMeter)
#define ChineseHint_Char_6P12P_Number  17
#define ChineseHint_Char_12P12P_Number 56
#define GBKNumber (ChineseHint_Char_6P12P_Number+ChineseHint_Char_12P12P_Number)
#else
#define ChineseHint_Char_7P14P_Number  17
#define ChineseHint_Char_14P14P_Number 56
#define GBKNumber (ChineseHint_Char_7P14P_Number+ChineseHint_Char_14P14P_Number)
#endif

/*变量声明--------------------------------------------------------------------*/

#if (MeterType == ThreePhaseMeter)
//象限区专用字库数组
//带圈 Ⅰ(0) 带圈 Ⅱ(1)带圈  Ⅲ(2) 带圈 Ⅳ(3)
extern const unsigned char QuadrantIcon_12p12p[4][24];

//中文提示区数字和字母专用字库不包含罗马数字ⅠⅡⅢⅣ
/*
 0(0) 1(1) 2(2) 3(3) 4(4) 5(5) 6(6) 7(7) 8(8) 9(9) T(10) A(11) B(12) C(13)
 N(14) O(15) S(16)
 */
extern const unsigned char ChineseHint_Char_6p12p[ChineseHint_Char_6P12P_Number][12];

//中文提示区汉字专用字库包含罗马数字ⅠⅡⅢⅣ
/*
 当(0) 前(1) 上(2) 月(3) 组(4) 合(5) 正(6) 反(7) 向(8) 总(9) 尖(10) 峰(11) 平(12) 谷(13) 剩(14) 余(15)
 常(16) 数(17) 阶(18) 梯(19) 透(20) 支(21) 用(22) 电(23) 量(24) 价(25) 户(26) 时(27) 间(28) 段(29) 金(30) 额(31)
 表(32) 号(33) 费(34) 率(35) 无(36) 有(37) 功(38) 需(39) 失(40) 压(41) 流(42)  率(43)  Ⅰ(44) Ⅱ(45) Ⅲ(46) Ⅳ(47)
 日(48) 期(49) 视(50) 在(51) 相(52) 第(53) 象(54) 限(55)
 */
extern const unsigned char ChineseHint_Char_12p12p[ChineseHint_Char_12P12P_Number][24];

//分屏序号专用字库，00~09
extern const unsigned char SplitScreenIcon_13p12p[10][26];
#else
//中文提示区数字和字母专用字库不包含罗马数字ⅠⅡⅢⅣ
/*
 0(0) 1(1) 2(2) 3(3) 4(4) 5(5) 6(6) 7(7) 8(8) 9(9) T(10) A(11) B(12) C(13)
 N(14) O(15) S(16)
 */
extern const unsigned char ChineseHint_Char_7p14p[ChineseHint_Char_7P14P_Number][14];

//中文提示区汉字专用字库包含罗马数字ⅠⅡⅢⅣ
/*
 当(0) 前(1) 上(2) 月(3) 组(4) 合(5) 正(6) 反(7) 向(8) 总(9) 尖(10) 峰(11) 平(12) 谷(13) 剩(14) 余(15)
 常(16) 数(17) 阶(18) 梯(19) 透(20) 支(21) 用(22) 电(23) 量(24) 价(25) 户(26) 时(27) 间(28) 段(29) 金(30) 额(31)
 表(32) 号(33) 费(34) 率(35) 无(36) 有(37) 功(38) 需(39) 失(40) 压(41) 流(42)  率(43)  Ⅰ(44) Ⅱ(45) Ⅲ(46) Ⅳ(47)
 日(48) 期(49) 视(50) 在(51) 相(52) 第(53) 象(54) 限(55)
 */
extern const unsigned char ChineseHint_Char_14p14p[ChineseHint_Char_14P14P_Number][28];

//分屏序号专用字库，00~09
extern const unsigned char SplitScreenIcon_14p14p[10][28];
#endif

//数字区专用“-”（0）
extern const unsigned char MinusIcon_8p36p[1][40];
//数字区专用小数点“.”(0),专用冒号“：”(1)
extern const unsigned char Point_4p36p[2][20];
//数字区专用数字 0(0) 1(1) 2(2) 3(3) 4(4) 5(5) 6(6) 7(7) 8(8) 9(9)
extern const unsigned char NumberArea_char_18p36p[10][90];


// 单位区专用汉字：万(0) 元(1)
extern const unsigned char UintArea_char_12p12p[2][24];
//用这些字符，组成：W,V,A,kW，kWh，var，VA，kVA,Ah;
// 单位区专用字符： W(0) V(1) A(2) k(3) h(4) v(5) a(6) r(7)
extern const unsigned char UintArea_char_8p12p[8][16];
// 单位区专用字符：kvar(0) kvah(1)
extern const unsigned char UintArea_char_24p12p[2][48];
// 单位区专用字符：kvarh(0)
extern const unsigned char UintArea_char_25p12p[1][50];


//报警区专用字符： 读(0) 卡(1) 成(2) 功(3) 请(4) 购(5) 电(6) 拉(7) 闸(8) 透(9) 支(10) 逆(11) 相(12) 序(13)
extern const unsigned char AlarmHint_char_12p12p[14][24];

#if (MeterType == ThreePhaseMeter)
//状态提示区专用字符：闭锁状态(0)
extern const unsigned char StatusHint_char_6p11p[1][12];

//状态提示区专用字符：公私钥状态(0) 第一路485通信状态(1) 第二路485通信状态(2) 电池1状态(3) 电池2状态(4)
extern const unsigned char StatusHint_char_7p11p[5][14];

//状态提示区专用字符：Ub(0) Uc(1)
extern const unsigned char StatusHint_char_8p11p[2][16];

//状态提示区专用字符：编程状态(0) Ua(1)
extern const unsigned char StatusHint_char_9p11p[2][18];

//状态提示区专用字符：正Ia状态(0) 负Ia状态(1) 
extern const unsigned char StatusHint_char_11p11p[2][22];

//状态提示区专用字符：载波状态(0) 正Ib状态(1) 负Ib状态(2) 正Ic状态(3) 负Ic状态(4)
extern const unsigned char StatusHint_char_10p11p[5][20];

//状态提示区专用字符：无线通信在线无信号(0) 无线通信在线1格信号(1) 无线通信在线2格信号(2) 无线通信在线3格信号(3) 无线通信在线4格信号(4) 报警指示(5)
extern const unsigned char StatusHint_char_12p11p[6][24];

/*
状态提示区专用字符：
费率1(0) 费率2(1) 费率3(2) 费率4(3) 费率5(4) 费率6(5) 费率7(6) 费率8(7) 费率9(8) 费率10(9) 费率11(10) 费率12(11) 费率13(12) 费率14(13) 费率15(14) 费率16(15)
费率17(16) 费率18(17) 费率19(18) 
阶梯1(19) 阶梯2(20) 阶梯3(21) 阶梯4(22) 阶梯5(23) 阶梯6(24) 阶梯7(25) 阶梯8(26) 阶梯9(27) 阶梯10(28) 阶梯11(29) 阶梯12(30) 阶梯13(31) 阶梯14(32) 阶梯15(33) 阶梯16(34) 
阶梯17(35) 阶梯18(36) 阶梯19(37) 
*/
extern const unsigned char StatusHint_char_13p11p[38][26];

#else
//状态提示区专用字符：485和红外状态(0)
extern const unsigned char StatusHint_char_6p11p[1][12];

//状态提示区专用字符：闭锁状态(0) 电池状态(1)
extern const unsigned char StatusHint_char_7p11p[2][14];

//状态提示区专用字符：反向状态(0) 
extern const unsigned char StatusHint_char_9p11p[1][18];

//状态提示区专用字符：载波状态(0) 公私钥状态(1) 
extern const unsigned char StatusHint_char_10p11p[2][20];

//状态提示区专用字符：编程状态(0) 
extern const unsigned char StatusHint_char_13p11p[1][26];

/*
状态提示区专用字符：
费率1(0) 费率2(1) 费率3(2) 费率4(3) 费率5(4) 费率6(5) 费率7(6) 费率8(7) 费率9(8) 费率10(9) 费率11(10) 费率12(11) 费率13(12) 费率14(13) 费率15(14) 费率16(15)
费率17(16) 费率18(17) 费率19(18) 
阶梯1(19) 阶梯2(20) 阶梯3(21) 阶梯4(22) 阶梯5(23) 阶梯6(24) 阶梯7(25) 阶梯8(26) 阶梯9(27) 阶梯10(28) 阶梯11(29) 阶梯12(30) 阶梯13(31) 阶梯14(32) 阶梯15(33) 阶梯16(34) 
阶梯17(35) 阶梯18(36) 阶梯19(37) 
*/
extern const unsigned char StatusHint_char_14p11p[38][28];
#endif 
/* GBK码和字符对应的数组 */
extern const GBK_CHARADDRESS_STRUCT  charaddressbuf[GBKNumber];


#endif
/*end------------------------------------------------------------------------*/

