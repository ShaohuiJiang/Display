/*******************************************************************************
* @file      : Display.h
* @author    : Jiangsh
* @version   : V1.0.0
* @date      : Sat Apr 21 2018
* @brief     : 
********************************************************************************
* @attention :������ı��д��ں��ֱ������ı�Ҫת����GB2312�����ʽ����ʹ�
*
*
*/
/* ��������-------------------------------------------------------------------*/
#ifndef _DISPLAY_H
#define _DISPLAY_H



/*ͷ�ļ�----------------------------------------------------------------------*/
///���ͷ�ļ�
#include "LCDConfig.h"

/*�궨��----------------------------------------------------------------------*/
///��Ӻ궨��
typedef enum
{
    CombinedActivePowerEnergy,          //����й�����
	PositiveActivePowerEnergy,	        //�����й�����
	ReverseActivePowerEnergy,		    //�����й�����
    CombinedOneReactivePowerEnergy,     //���1�޹�����
    CombinedTwoReactivePowerEnergy,     //���2�޹�����
	PositiveReactivePowerEnergy,	    //�����޹�����
	ReverseReactivePowerEnergy,		    //�����޹�����
    FirstQuadrantReactivePowerEnergy,   //��1�����޹�����
    SecondQuadrantReactivePowerEnergy,  //��2�����޹�����
    ThirdQuadrantReactivePowerEnergy,   //��3�����޹�����
    FourthQuadrantReactivePowerEnergy,  //��4�����޹�����
    PositiveApparentEnergy,             //�������ڵ���
    ReverseApparentEnergy               //�������ڵ���
}ENERGY_TYPE;                           //��������

typedef enum
{
    TotalPhase,                         //����
    APhase,                             //A��
    BPhase,                             //B��
    CPhase                              //C��
}PHASE_TYPE;                            //��λ����


typedef enum
{
    Plus,                      //��
    Minus                      //��
}PLUS_MINUS;                   //������־


typedef enum
{
    DisplayHighZero,           //��ʾ��λ��
    NoDisplayHighZero          //����ʾ��λ��
}HIGHZERO_TYPE;                //��λ�����־

typedef enum
{
    NoDisplaySplitScreen,               //����ʾ����
    DisplaySplitScreenZero,             //��ʾ�������00
    DisplaySplitScreenOne,              //��ʾ�������01
    DisplaySplitScreenTwo,              //��ʾ�������02
    DisplaySplitScreenThree             //��ʾ�������03
    //���Լ�������...
}SPLITSCREENDISPLAY_TYPE;               //������ʾ��־

/*�����ⲿ����----------------------------------------------------------------*/
/** 
 * @brief  ��ʾ��������
 * @note  
 * @param  phase: ������λ ����μ�PHASE_TYPEö�� 
 * @param  engerytype: �������࣬����μ�ENERGY_TYPEö��
 * @param  date:    ���ڣ���ʱ֧��0~12��0��ʾ��ǰ  ����������x��
 * @param  rate:    ���� 0~12������0�����ܣ���������Tx
 * @param  engerypoint: ָ������洢�����飬Ĭ��6�ֽ�BCD�룬����ֽڴ����3,4С��
 * @param  decimalpoint: ������ʾ�ĵ�����ʾ��λС����0~4
 * @param  plusminus: �����Ƿ���ʾ���ţ�    Plus������ʾ��Minus������ʾ
 * @param  displayhighzero: ��λ�Ƿ����㣬����μ�HIGHZERO_TYPEö��
 * @retval None
 */
extern void Display_Engery(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate,unsigned char* engerypoint,unsigned char decimalpoint,PLUS_MINUS plusminus,HIGHZERO_TYPE displayhighzero);

/** 
 * @brief  ��ʾ��ǰ����
 * @note   
 * @param  datepoint: ָ������洢�����飬Ĭ��3�ֽ�BCD�룬����ֽڴ�����
 * @retval None
 */
extern void Display_CurrentDate(unsigned char* datepoint);

/** 
 * @brief  ��ʾ��ǰʱ��
 * @note   
 * @param  timepoint: ָ��ʱ��洢�����飬Ĭ��3�ֽ�BCD�룬����ֽڴ�����
 * @retval None
 */
extern void Display_CurrentTime(unsigned char* timepoint);

/** 
 * @brief  ��ʾ��ǰʣ����
 * @note   ������ʾ����ǰʣ���ѡ� ������ʾ����ǰʣ���
 * @param  amountpoint: ָ��ʣ����洢�����飬Ĭ��4�ֽ�BCD�룬����ֽڴ���С��1��2λ
 * @param  displayhighzero: ��λ�Ƿ����㣬0������ʾ�� 1������ʾ
 * @retval None
 */
extern void Display_RemainingAmount(unsigned char* amountpoint,unsigned char displayhighzero);

/** 
 * @brief  ��ʾ��ǰ͸֧���
 * @note   ������ʾ����ǰ͸֧��ѡ� ������ʾ����ǰ͸֧��
 * @param  amountpoint: ָ��͸֧���洢�����飬Ĭ��4�ֽ�BCD�룬����ֽڴ���С��1��2λ
 * @param  displayhighzero: ��λ�Ƿ����㣬0������ʾ�� 1������ʾ
 * @retval None
 */
extern void Display_OverdraftAmount(unsigned char* amountpoint,unsigned char displayhighzero);


extern unsigned char  Get_GBKCodeOfStr(unsigned char* strpoint,unsigned short* gbkbufpoint);

extern void Fill_Char_In_ChineseHintArea(unsigned char* strbuf,SPLITSCREENDISPLAY_TYPE SplitScreen);
extern void InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear);
extern void Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf(unsigned char len,unsigned short offset);
extern void Clear_ChineseHintArea_LCDRAM_BackupBuf(void);
#endif
/*end------------------------------------------------------------------------*/
