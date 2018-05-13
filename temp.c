/*******************************************************************************
* @file      : temp.c
* @author    : Jiangshaohui
* @version   : V1.0.0
* @date      : Sun May 13 2018
* @brief     : 
********************************************************************************
* @attention :
*
*
*/
/*ͷ�ļ�----------------------------------------------------------------------*/
///���ͷ�ļ�
#include "Display.h"
#include "string.h"


/*�궨��----------------------------------------------------------------------*/
///��Ӻ궨��
#define ChineseHintAreaGBKBufSize 20                    //������ʾ��GBK�뻺�����ߴ�
#define ChineseHintAreaLCDRAMBackupBufSegSize   320     //������ʾ�����õ��󻺴����ߴ�

/*�ڲ���������----------------------------------------------------------------*/
///����ڲ�����
static short ChineseHintAreaGBKBuf[ChineseHintAreaGBKBufSize];//������ʾ��GBK�뻺����
static unsigned char ChineseHintAreaLCDRAMBackupBuf[ChineseHintAreaLCDRAMBackupBufSegSize][ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom];  //������ʾ�����õ��󻺴���

static unsigned char  ChineseHintArea_RollDisplay;      //������ʾ���������Ƿ����ö�̬��ʾ��־��0�����ã�1�Ǵ������ã�ע�⣬��ʼ��ʱĬ�����ã�ÿ����������ʾ��ʱ������ */
/*�ڲ�������ʼ��--------------------------------------------------------------*/
///����ڲ�����
static 
/*�����ڲ�����----------------------------------------------------------------*/



//����������ʾ��GBK�뻺�����ĺ���
static void Clear_ChineseHintAreaGBKBuf(void);
static void Wirte_ChineseHintAreaGBKBuf(short gbkcode);
static short Read_ChineseHintAreaGBKBuf(unsigned char index);
static unsigned char Read_GBKNum_ChineseHintAreaGBKBuf(void);
static void StrToChineseHintAreaGBKBuf(char *str);

//����������ʾ�����õ��󻺴����ߴ�
static void Clear_ChineseHintAreaLCDRAMBackupBuf(void);
static void ChineseHintAreaLCDRAMBackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit);
static void InputCharacter_to_ChineseHintAreaLCDRAMBackupBuf(unsigned short x,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear);
static unsigned short ChineseHintAreaGBKBufToChineseBackupBuf(void);

static void Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf(unsigned short endseg,unsigned short offset);

static void Fill_Char_In_ChineseHintArea(unsigned char* strbuf,char SplitScreen);
/*�����ڲ�����----------------------------------------------------------------*/
///����ֻ���ڱ��ļ�ʹ�õĺ���


static void Fill_Char_In_ChineseHintArea(unsigned char* strbuf,char SplitScreen)
{
    unsigned char i;
    unsigned short endseg;
    unsigned short len;
    const unsigned char* addresspoint;
    unsigned int sizenumber;
    unsigned int size;
    char tempstr[ChineseHintAreaGBKBufSize*2];
    char Twospacebuf[] = "  ";
    ChineseHintArea_RollDisplay = 0;        //ֹͣ������ʾ
    Clear_ChineseHintAreaGBKBuf();          //���
    Clear_ChineseHintAreaLCDRAMBackupBuf(); //���

    //����ȷ���Ƿ���ʾ��������ȷ��������ʾ��������ʾ�ķ�Χ
    //�жϷ����Ƿ���ʾ
    if(SplitScreen < 0)                    //����ʾ����
    {
        endseg = ChineseHintAreaEndSeg;
    }
    else if(SplitScreen < 9)              //��ʾ����
    {
        #if (MeterType == ThreePhaseMeter)
        addresspoint = &SplitScreenIcon_13p12p[SplitScreen][0];
        size = Size_13P12P;
        #else
        addresspoint = &SplitScreenIcon_14p14p[SplitScreen][0];
        size = Size_14P14P;
        #endif

        endseg = SplitWindowAreaStartseg;   //��ʾ�����ˣ�������ʾ��������ʾ�ķ�Χ����С��
    }

    StrToChineseHintAreaGBKBuf(strbuf);     //���ַ���ת��GBK�벢����ChineseHintAreaGBKBuf��
    len = ChineseHintAreaGBKBufToChineseBackupBuf();  //����GBK��Ѱ�ҵ��������ݲ�д��ChineseBackupBuf��
    Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf(endseg,0);  //����������ʾ��������ʾ�ķ�Χ��ChineseBackupBuf��ʼ��ַ���Ƶ�ChineseHintArea_LCDRAM

    if(len > (endseg-ChineseHintAreaStartSeg))          //Ҫ��ʾ������д����
    {
        sprintf(tempstr,"%s%s",strbuf,Twospacebuf);     //��ԭ�ȵ��ַ���ĩβ���2���ո񣬼���xx����ɡ�xx  ��
        StrToChineseHintAreaGBKBuf(tempstr);            //���ַ���ת��GBK�벢����ChineseHintAreaGBKBuf��
        len = ChineseHintAreaGBKBufToChineseBackupBuf();//����GBK��Ѱ�ҵ��������ݲ�д��ChineseBackupBuf��
        ChineseHintArea_RollDisplay = 1��               //���ù�����ʾ
    }
}

/** 
 * @brief  ������������д��������ʾ��
 * @note   
 * @param  phase: ��λ
 * @param  engerytype: ��������
 * @param  date: ʱ��
 * @param  rate: ����
 * @retval None
 */
static void Fill_Engery_In_ChineseHintArea(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate)
{
    char strbuf[100] = "";
    char strbuf2[20] = "";
    int num;

    if((date>99)||rate>99)    //�·ݳ���99�£�С���㳬��4λ���߷��ʳ���99��֧�֣�����
    {
        return;
    }
    //ȷ��ʱ��
    if(date == 0)       //��ǰ
    {
        strcat(strbuf,"��ǰ");
    }
    else                //��x��
    {
        strcat(strbuf,"��");
        num  = (int)date;
        strcat(strbuf2,"��");
        sprintf(strbuf,"%s%d%s",strbuf,num,strbuf2);
    }

    //ȷ����λ
    switch(phase)
    {
        case TotalPhase:
            strcat(strbuf,"");
        break;
        case APhase:
            strcat(strbuf,"A��");
        break;
        case BPhase:
            strcat(strbuf,"B��");
        break;
        case CPhase:
            strcat(strbuf,"C��");
        break;
        default:
        break;
    }
    //ȷ����������
    switch(engerytype)
    {
        case CombinedActivePowerEnergy:
            #if (MeterType == ThreePhaseMeter)
            strcat(strbuf,"����й�");
            #else
            strcat(strbuf,"�й�");
            #endif
        break;
        case PositiveActivePowerEnergy:
            strcat(strbuf,"�����й�");
        break;
        case ReverseActivePowerEnergy:
            strcat(strbuf,"�����й�����");
        break;
        case CombinedOneReactivePowerEnergy:
            strcat(strbuf,"����޹�1");
        break;
        case CombinedTwoReactivePowerEnergy:
            strcat(strbuf,"����޹�2");
        break;
        case ReverseReactivePowerEnergy:
            strcat(strbuf,"�����޹�");
        break;
        case FirstQuadrantReactivePowerEnergy:
            strcat(strbuf,"��1�����޹�");
        break;
        case SecondQuadrantReactivePowerEnergy:
            strcat(strbuf,"��2�����޹�");
        break;
        case ThirdQuadrantReactivePowerEnergy:
            strcat(strbuf,"��3�����޹�");
        break;
        case FourthQuadrantReactivePowerEnergy:
            strcat(strbuf,"��4�����޹�");
        break;
        case PositiveApparentEnergy:
            strcat(strbuf,"��������");
        break;
        case ReverseApparentEnergy:
            strcat(strbuf,"��������");
        break;
        default:
            strcat(strbuf,"");
        break;
    }
    //ȷ������
    if(rate == 0)
    {
        strcat(strbuf,"��");
    }
    else
    {
        strcat(strbuf,"T");
        num = (int)rate;
        sprintf(strbuf,"%s%d",strbuf,num);
    }

    //���д��������
    strcat(strbuf,"����");
    //ˢ��LCD_RAM��backupbuf�ĵ�������
    Fill_Char_In_ChineseHintArea(strbuf,-1);
}

/*����ȫ�ֺ���----------------------------------------------------------------*/
///����������ⲿ�ļ��ĺ���














/*end-------------------------------------------------------------------------*/