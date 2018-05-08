/*******************************************************************************
* @file      : Display.c
* @author    : Jiangsh
* @version   : V1.0.0
* @date      : Sat Apr 21 2018
* @brief     : 
********************************************************************************
* @attention :������ı��д��ں��ֱ������ı�Ҫת����GB2312�����ʽ����ʹ�
*
*
*/
/*ͷ�ļ�----------------------------------------------------------------------*/
///���ͷ�ļ�
#include "Display.h"
#include "LCD.h"
#include "CharLib.h"
#include "LCDConfig.h"

/*�궨��----------------------------------------------------------------------*/
///��Ӻ궨��
#define ChineseHintAreaGBKBufSize 20    //������ʾ���ַ�GBK�뻺�����Ĵ�С
/*�ڲ���������----------------------------------------------------------------*/
///����ڲ�����
//static unsigned char charisoverflow;            //�ַ�����������������ʾ��
//static unsigned char ChineseHintAreaCharBuf;    //������ʾ���ַ���������
static unsigned short ChineseHintAreaGBKBuf[ChineseHintAreaGBKBufSize];  //������ʾ���ַ�GBK�뻺����
static unsigned char  ChineseHintAreaGBKLen;                             //������ʾ����ЧGBK������
/* ������ʾ��LCD�ı��ݻ������飬�������ҹ�����ʾ ��С��������ʾ�����������2�� */
static unsigned char ChineseHintArea_LCDRAM_BackupBuf[(ChineseHintAreaEndSeg-ChineseHintAreaStartSeg)*2][ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom];

static unsigned char  ChineseHintArea_RollDisplay;


/*�ڲ���������----------------------------------------------------------------*/
///����ڲ�����




/*�����ڲ�����----------------------------------------------------------------*/
///����ֻ�ڱ��ļ�ʹ�õĺ���
static void  Display_SplitScreen_Number(unsigned char number);

static unsigned char  Get_InvalidZero_Number(unsigned char* buf,unsigned char len);

static void  Adjust_DecimalpointOfValue(unsigned char* srcbuf,unsigned char* objbuf,unsigned char decimalpoint);

//static void Clear_ChineseHintArea_LCDRAM_BackupBuf(void);
//static void  Get_GBKCodeOfStr(unsigned char* strpoint,unsigned short* gbkbufpoint,unsigned char* charnumberpoint);

//static unsigned int GBK_to_CharBufAddress(unsigned short gbkcode,unsigned char* charbufaddress); 

const unsigned char* Get_CharBufAddress(unsigned short gbkcode);
//static void ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit);
//static void InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear);


static unsigned char Fill_CurrentAndLastXMonth_In_ChineseHintArea(unsigned char startseg,unsigned char date);
static unsigned char Fill_Phase_In_ChineseHintArea(unsigned char startseg,PHASE_TYPE phase);
static unsigned char Fill_EngeryType_In_ChineseHintArea(unsigned char startseg,ENERGY_TYPE engerytype);
static unsigned char Fill_EngeryRate_In_ChineseHintArea(unsigned char startseg,unsigned char rate);
static void Fill_Engery_In_ChineseHintArea(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate);
static void Fill_CurrentDate_In_ChineseHintArea(void);
static void Fill_CurrentTime_In_ChineseHintArea(void);
static void Fill_RemainingAmount_In_ChineseHintArea(void);
static void Fill_OverdraftAmount_In_ChineseHintArea(void);

static void Fill_Value_In_NumberArea(unsigned char* valuepoint,unsigned char decimalpoint,PLUS_MINUS plusminus,HIGHZERO_TYPE displayhighzero);
static void Fill_Date_In_NumberArea(unsigned char* datepoint);
static void Fill_Time_In_NumberArea(unsigned char* timepoint);
static void Fill_Amount_In_NumberArea(unsigned char* amountpoint,PLUS_MINUS plusminus,HIGHZERO_TYPE displayhighzero);

static void Fill_Kwh_In_UnitArea(void);
static void Fill_kvah_In_UnitArea(void);
static void Fill_Yuan_In_UnitArea(void);


/*�����ڲ�����----------------------------------------------------------------*/
///����ֻ���ڱ��ļ�ʹ�õĺ���
/** 
 * @brief  
 * @note   
 * @param  number: 
 * @retval None
 */
static void Display_SplitScreen_Number(unsigned char number)
{
    unsigned int size;
    unsigned char segpoint;
    
    #if (MeterType == ThreePhaseMeter)
    size = Size_13P12P;
    #else
    size = Size_14P14P;
    #endif   
    
    if(number>19)       //���һ����ʾ��������19��
    {
        return;
    }
    segpoint = SplitWindowAreaStartseg;
    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&SplitScreenIcon_13p12p[number][0],size,display);   //�������
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&SplitScreenIcon_14p14p[number][0],size,display);   //�������
    #endif
}

/** 
 * @brief  ��ȡ�����и�λ��Ч0�ĸ���
 * @note   �������ΪBCD��ʽ����Ĭ�ϵ��ֽ������ŵ�λ����
 * @param  buf: ���жϵ�������ʼ��ַ
 * @param  len: ���жϵ������ֽڳ���  0~10
 * @retval ��Ч0��ʽ
 */
static unsigned char  Get_InvalidZero_Number(unsigned char* buf,unsigned char len)
{
        unsigned char temp;
        unsigned char i;
        unsigned char number;

        if(len>10)          //�����ˣ�����
        {
            return 255;
        }

        number = 0;

        for(i=0;i<len;i++)          //�������λ�ֽ����ݲ��ж��Ƿ�Ϊ0����ѭ��
        {
            temp = *(buf+len-1-i); 
  
            if((temp&0xf0) == 0x00)   //���жϸ�λ������0������Ч0����1�������ֱ�ӷ���
            {
                number++;
            }
            else                    //�������㣬�����жϣ�������
            {

                return number;
            }            
            if((temp&0x0f) == 0x00)   //���жϵ�λ������0������Ч0����1�������ֱ�ӷ���
            {
                number++;
            }
            else                    //�������㣬�����жϣ�������
            {

                return number;
            }

        }

        return number;              //ѭ�����ˣ�Ҳ����
}

/** 
 * @brief  ����4λС����6�ֽ����鱣����С��λ���Ƶ�Ŀ������
 * @note   ע��Դ����ĸ�ʽ����������6�ֽڣ���Ĭ������ֽ�2������Ԫ��ΪС��
 *         �ú���ֻҪ��Ϊ�˴���4λС���ĵ���
 * @param  srcbuf: 6�ֽ����飬����ֽ�2������Ԫ��ΪС����BCD��ʽ
 * @param  objbuf: 4�ֽ����飬BCD��ʽ
 * @param  decimalpoint: ׼��������С��������0~4
 * @retval None
 */
static void  Adjust_DecimalpointOfValue(unsigned char* srcbuf,unsigned char* objbuf,unsigned char decimalpoint)
{
    
    if(decimalpoint>4)  //����Χ�ˣ�ֱ�ӷ���
    {
        return;
    }

    switch(decimalpoint)//����С��λ����Ŀ������
    {
        case 0:             //��С��λ
        {
            *(objbuf)   = *(srcbuf+2);
            *(objbuf+1) = *(srcbuf+3);
            *(objbuf+2) = *(srcbuf+4);
            *(objbuf+3) = *(srcbuf+5);
        }
        break;
        case 1:             //1λС��λ
        {
            *(objbuf)   = ((*(srcbuf+1)>>4)&0x0f)|((*(srcbuf+2)<<4)&0xf0);
            *(objbuf+1) = ((*(srcbuf+2)>>4)&0x0f)|((*(srcbuf+3)<<4)&0xf0);
            *(objbuf+2) = ((*(srcbuf+3)>>4)&0x0f)|((*(srcbuf+4)<<4)&0xf0);
            *(objbuf+3) = ((*(srcbuf+4)>>4)&0x0f)|((*(srcbuf+5)<<4)&0xf0);   
        }
        break;

        case 2:             //2λС��λ
        {

            *(objbuf)   = *(srcbuf+1);
            *(objbuf+1) = *(srcbuf+2);
            *(objbuf+2) = *(srcbuf+3);
            *(objbuf+3) = *(srcbuf+4);
        }
        break;

        case 3:             //3λС��λ
        {
            *(objbuf)   = ((*(srcbuf+0)>>4)&0x0f)|((*(srcbuf+1)<<4)&0xf0);
            *(objbuf+1) = ((*(srcbuf+1)>>4)&0x0f)|((*(srcbuf+2)<<4)&0xf0);
            *(objbuf+2) = ((*(srcbuf+2)>>4)&0x0f)|((*(srcbuf+3)<<4)&0xf0);
            *(objbuf+3) = ((*(srcbuf+3)>>4)&0x0f)|((*(srcbuf+4)<<4)&0xf0);

        }
        break;
        case 4:             //4λС��λ
        {
            *(objbuf)   = *(srcbuf);
            *(objbuf+1) = *(srcbuf+1);
            *(objbuf+2) = *(srcbuf+2);
            *(objbuf+3) = *(srcbuf+3);
        }       
        break;      
        default:
        break;
    }   
}
/** 
 * @brief  ���������ʾ�����ݻ�����������
 * @note   
 * @retval None
 */
extern void Clear_ChineseHintArea_LCDRAM_BackupBuf(void)
{
    unsigned short i,j;

    for(i=0;i<(ChineseHintAreaEndSeg-ChineseHintAreaStartSeg)*2;i++)
    {
        for(j=0;j<(ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom);j++)
        {
            ChineseHintArea_LCDRAM_BackupBuf[i][j] = 0;
        }
    }
   
}
/*����LCDRAM_BackupBuf���麯��------------------------*/
/** 
 * @brief  LCDRAM_BackupBuf����λ����㺯��
 * @note   (x,y)λ�õ�ĳ�㣬��ʾ���߲���ʾ��
 * ע�⣬���ֻ�����˱��û������ĵ㣬������ʾ��Ҫ�����������������û�����д�����������ٽ�����������д��LCD
 * @param  x: seg��λ��
 * @param  y: com��λ��
 * @param  bit: 0������ʾ��1����ʾ
 * @retval None
 */
extern  void ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit)
{
    unsigned short pos;
    unsigned short bx;
    unsigned char temp=0;

    if((x>=(seg*2))||y>=com)return;       //������Χ
    pos=y/8;        //�õ�ҳ��ַ       
    
    bx=y%8;         //�õ�����ҳ��ַ��bitλ
    temp=1<<(7-bx);

    if(bit)
    {
        ChineseHintArea_LCDRAM_BackupBuf[x][pos]|=temp;
    }
    else 
    {
        ChineseHintArea_LCDRAM_BackupBuf[x][pos]&=~temp; 
    }   
}
/** 
 * @brief  ��ChineseHintArea_LCDRAM_BackupBuf��ָ��λ����ʾ�����һ���ַ�(�������֡���ĸ�����֡�����)
 * @note   ��㷽ʽ������ʽ������Ļ���ȷ��ͬһλ����ʾ�˶�Ӧ�ַ�����Ȼ����֤���Ч��
 * @param  x: ָ����ʼλ�õ�seg
 * @param  y: ָ����ʼλ�õ�com
 * @param  charbufstartaddress: �ַ�����ģ��ʼ��ַ
 * @param  size: �ַ��ߴ�
 * @param  displayorclear: clear���������display������ʾ
 * @retval None
 */

extern  void InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear)
{
    unsigned char temp;
    unsigned short y0;
    unsigned int t,t1,segnumber,comnumber,bytesnumber; 

    y0 = y;
    segnumber = size/100;
    comnumber = size%100;
    bytesnumber = segnumber*(comnumber/8+((comnumber%8)?1:0));      //�õ���Ҫ��ʾ���õ��ֽ���
    
    for(t=0;t<bytesnumber;t++)                                      //����ʾ���õ��ֽ�ȫ��д��LCDRAM_Buf������
    {     
        temp=*(charbufstartaddress+t);
        for(t1=0;t1<8;t1++) 
        {
            if(temp&0x80)
            {
 
                if((x>=(seg*2))||(y>=com))  //����Χ��
                {
                    return;                 //�������鷶Χ�ģ���д
                }
                if(displayorclear == display)      //д1������ʾ
                {
                    ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(x,y,1);
                }
                else                                //д0�������
                {
                    ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(x,y,0);
                }

            }
            temp<<=1;
            y++;
            if((y-y0)==(unsigned short)(size%100))
            {
                y=y0; x++;
                break;
            }
        }
    }
}

extern void Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf(unsigned char len,unsigned short offset)
{
    unsigned short i;
    unsigned short j;
    unsigned char temp;
    for(i=0;i<len;i++)
    {
        for(j=0;j<(ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom);j++)
        {
            temp = ChineseHintArea_LCDRAM_BackupBuf[i+offset][j];  
            Wirte_LCDRAM_Buf((ChineseHintAreaStartSeg+i),(ChineseHintAreaStartPageCom+j),temp); 
        }

    }    
}
/** 
 * @brief   ���ַ���ת����GBK��
 * @note   
 * @param  strpoint: �ַ���ָ��
 * @param  gbkbufpoint: ת���ɵ�GBK��������ĵ�ַָ��
 * @retval GBK������
 */
extern unsigned char  Get_GBKCodeOfStr(unsigned char* strpoint,unsigned short* gbkbufpoint)
{
    unsigned char i;
    unsigned char charnumberpoint=0;
    
    for(i=0;i<42;i++)           //��ֵ�ַ�����GBK���������ת�棬ͬʱ��������
    {
        if(*(strpoint+i) == 0)  //����\n �����Ѿ�����
        {
           return charnumberpoint;              //�˳�
        }
        else if(((*(strpoint+i))&0x80) == 0x80)      //���λΪ1�������Ǻ��ֱ���һ����
        {
            i++;                            
            if(((*(strpoint+i))&0x80) == 0x80)      //���λΪ1�������Ǻ��ֱ���һ����
            {
               *gbkbufpoint =  (*(strpoint+i-1))*256+(*(strpoint+i));       //�õ�2�ֽڵ�GBK��
            }
            else                           //ǰһ���Ǻ��ֱ���һ���֣���һ�����ǣ������Ϲ��ɣ�������
            {
                continue;                  //ֹͣ��һ�Σ�������һ��ѭ��
            }            
        }
        else                              //���λ����1���Ҳ���\n �ʹ�������ĸ��������
        {
            *gbkbufpoint =  *(strpoint+i); //�õ�2�ֽڵ�GBK�룬��λ�ֽ�Ϊ00
        }        
        //��ִ�е����˵��һ���ַ������ɹ�
        gbkbufpoint++;                      //GBK��������ָ���1
        charnumberpoint++;                  //��Ч�ַ�����1 
    }

    return charnumberpoint;              //�˳�

}

/** 
 * @brief  ����GBK�뷵���ַ��ֿ���������λ��
 * @note   
 * @param  gbkcode: gbk��
 * @retval ����λ�õ�ָ��
 */
const unsigned char* Get_CharBufAddress(unsigned short gbkcode)
{   
    unsigned char i;

    for(i=0;i<GBKNumber;i++)
    {
        if(gbkcode == charaddressbuf[i].CharGbk)
        {
            return charaddressbuf[i].CharAddress;
        }
    }
    //�Ҳ������ͷ�������0��λ��
    #if (MeterType == ThreePhaseMeter)
    return &ChineseHint_Char_6p12p[0][0];
    #else
    return &ChineseHint_Char_7p14p[0][0];    
    #endif
}


extern void Fill_Char_In_ChineseHintArea(unsigned char* strbuf,SPLITSCREENDISPLAY_TYPE SplitScreen)
{
    unsigned char i;
//    unsigned char j;
//  unsigned char len;
    const unsigned char* addresspoint;
    unsigned short segpoint;
    unsigned char segendpoint;
    unsigned int sizenumber;
    unsigned int size;
    
    //��ʼ��
    for(i=0;i<ChineseHintAreaGBKBufSize;i++)    
    {
        ChineseHintAreaGBKBuf[i] = 0;   //GBK�뻺��������
    }

    ChineseHintArea_RollDisplay = 0;                //������ʾ��־λ����
    ChineseHintAreaGBKLen = 0;                      //��ЧGBK����������
    Clear_ChineseHintArea_Of_LCDRAM_Buf();          //��ģ���ݻ���������
    Clear_ChineseHintArea_LCDRAM_BackupBuf();      //��ģ���ݱ��ݻ���������
    

    
    if(SplitScreen == NoDisplaySplitScreen)         //û�з�����������ʾ���ʹ󣬷���Ҫȥ���������ķ�Χ
    {
        segendpoint = ChineseHintAreaEndSeg;
    }
    else
    {
        segendpoint = SplitWindowAreaStartseg;
    }


    #if (MeterType == ThreePhaseMeter)
    sizenumber = Size_6P12P;
    size = Size_12P12P;
    #else
    sizenumber = Size_7P14P;
    size = Size_14P14P;
    #endif

    //�õ���ЧGBK�������
    ChineseHintAreaGBKLen = Get_GBKCodeOfStr(strbuf,ChineseHintAreaGBKBuf);

    //���ж���ЧGBK��������Ĭ��֧��ChineseHintAreaGBKBufSize��GBK�룬����������п���Clear_LCDRAM_BackupBuf�����ã���Ҫֱ�ӷ��أ�������ִ��
    if(ChineseHintAreaGBKLen>ChineseHintAreaGBKBufSize)
    {
        return;
    }
    else            //û�г������ͽ�����д�����û���������
    {
       
        segpoint =  0;
        
        for(i=0;i<ChineseHintAreaGBKLen;i++)
        {
            addresspoint = Get_CharBufAddress(ChineseHintAreaGBKBuf[i]);    //�õ��ַ����ڵ�ַ

            if(segpoint>(2*seg))        //�������û��������飬�ͷ���
            {
                break;
            }

            if((ChineseHintAreaGBKBuf[i]&0x8080) == 0x8080 )    //����
            {
               InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(segpoint,ChineseHintAreaStartCom,addresspoint,size,display);  

               //InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,addresspoint,size,display);   
               segpoint += size/100;    
            }
            else                                                //��ĸ������
            {

               InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(segpoint,ChineseHintAreaStartCom,addresspoint,sizenumber,display);   
               //InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,addresspoint,sizenumber,display);   
               segpoint += sizenumber/100;    
            }
        }  

        //��GBKȫ��ת������ģ���ݺ󣬾�Ҫ�ж�ȫ��������û�г�������ʾ����Χ�����糬�˾�Ҫ���ù�����ʾ
        Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf((segendpoint-ChineseHintAreaStartSeg),0);


        if(segpoint>=segendpoint)  //����
        {
            ChineseHintArea_RollDisplay =1;     //���ù�����ʾ
        }

    }
}

/** 
 * @brief  ��䡰��ǰ�����ߡ���x�¡���������ʾ��
 * @note   
 * @param  startseg: ��ʾ����ʼλ��
 * @param  date: ���ڣ���ʱ֧��0~12��0��ʾ��ǰ  ����������x��
 * @retval ���ص���д���seg����λ��
 */
static unsigned char Fill_CurrentAndLastXMonth_In_ChineseHintArea(unsigned char startseg,unsigned char date)
{
    unsigned int size;
    unsigned int sizenumber;
    unsigned char segpoint;
    unsigned char low,high;

    #if (MeterType == ThreePhaseMeter)
    sizenumber = Size_6P12P;
    size = Size_12P12P;
    #else
    sizenumber = Size_7P14P;
    size = Size_14P14P;
    #endif

    segpoint =  startseg;
    if(date == 0)           //��ʾ��ǰ       
    {
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //��
        segpoint += (size/100);
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //ǰ
        segpoint += (size/100);
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //��
        segpoint += (size/100);
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //ǰ
        segpoint += (size/100);
        #endif
    }
    else                   //��ʾ��X��
    {
        //��ȡ�·ݵĸߵ��ֽ�BCD��
        high=date/10;
        low=date%10;
       
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[2][0],size,display);   //��
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[2][0],size,display);   //��
        #endif
        segpoint += (size/100);

        //��ʾXX
        if(high != 0)       //��λ��Ϊ0������Ҫ��ʾ
        {           
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[high][0],sizenumber,display);   //x
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[high][0],sizenumber,display);   //x
            #endif
            segpoint += (sizenumber/100);
        }

        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[low][0],sizenumber,display);   //x
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[low][0],sizenumber,display);   //x
        #endif
        segpoint += (sizenumber/100);

        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[3][0],size,display);   //��
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[3][0],size,display);   //��
        #endif
        segpoint += (size/100);
    }

    return segpoint;
}

/** 
 * @brief  ��䡰A�ࡱ��B�ࡱ��C�ࡱ��������ʾ��
 * @note   ���಻��ʾ
 * @param  startseg: ��ʾ����ʼλ��
 * @param  phase: ��λ���ͣ�����μ�PHASE_TYPEö��
 * @retval ���ص���д���seg����λ��
 */
static unsigned char Fill_Phase_In_ChineseHintArea(unsigned char startseg,PHASE_TYPE phase)
{

    unsigned int size;
    unsigned int sizenumber;
    unsigned char segpoint;
    
    segpoint = startseg;
    
    #if (MeterType == ThreePhaseMeter)
    sizenumber = Size_6P12P;
    size = Size_12P12P;

    if(phase == APhase)
    {
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[11][0],sizenumber,display);   //A
        segpoint += (sizenumber/100);
    }
    else if(phase == BPhase)
    {
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[12][0],sizenumber,display);   //B
        segpoint += (sizenumber/100);
    }
    else if(phase == CPhase)
    {
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[13][0],sizenumber,display);   //C
        segpoint += (sizenumber/100);
    }
    if(phase != TotalPhase)
    {
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[52][0],size,display);   //��
        segpoint += (size/100);
    }
    #endif

    return segpoint;
}
/** 
 * @brief  ��䡰����й����������й����������й������޹����1�����޹����2���������޹����������޹������޹��񡱡��޹��򡱡��޹��󡱡��޹��������������ڡ����������ڡ���������ʾ��
 * @note   
 * @param  startseg: �ַ���ʾ��ʼλ��
 * @param  engerytype: �������࣬����μ�ENERGY_TYPEö��
 * @retval ���ص���д���seg����λ��
 */
static unsigned char Fill_EngeryType_In_ChineseHintArea(unsigned char startseg,ENERGY_TYPE engerytype)
{
    unsigned int size;
    unsigned int sizenumber;
    unsigned char segpoint;
    unsigned char combinedreactivepowerenergy;

    #if (MeterType == ThreePhaseMeter)
    sizenumber = Size_6P12P;
    size = Size_12P12P;
    #else
    sizenumber = Size_7P14P;
    size = Size_14P14P;
    #endif

    segpoint =  startseg;

    if((engerytype == CombinedActivePowerEnergy)||(engerytype == PositiveActivePowerEnergy)||(engerytype == ReverseActivePowerEnergy))     //�й���
    {

        if(engerytype == PositiveActivePowerEnergy)      //�����й���
        {           
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[6][0],size,display);   //��
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[6][0],size,display);   //��
            #endif
            segpoint += (size/100);             
        }
        else if(engerytype == ReverseActivePowerEnergy) //�����й���   
        {  
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[7][0],size,display);   //��
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[7][0],size,display);   //��
            #endif
            segpoint += (size/100);             
        }
        
        if(engerytype != CombinedActivePowerEnergy) //������ϣ�����ʾ����
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display);   //��
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display);     //��
            #endif
            segpoint += (size/100);  
        }

        #if (MeterType == ThreePhaseMeter)          //�������Ҫ��ʾ�й�
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[37][0],size,display); //��
        segpoint += (size/100); 
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[38][0],size,display);//��
        segpoint += (size/100);   
        #endif   
    }
    else if((engerytype == CombinedOneReactivePowerEnergy)||(engerytype == CombinedTwoReactivePowerEnergy)\
          ||(engerytype == PositiveReactivePowerEnergy)||(engerytype == ReverseReactivePowerEnergy)\
          ||(engerytype == FirstQuadrantReactivePowerEnergy)||(engerytype == SecondQuadrantReactivePowerEnergy)\
          ||(engerytype == ThirdQuadrantReactivePowerEnergy)||(engerytype == FourthQuadrantReactivePowerEnergy))     //�޹���
    {
        combinedreactivepowerenergy = 0;

        if((engerytype == CombinedOneReactivePowerEnergy)||(engerytype == CombinedTwoReactivePowerEnergy))      //����޹���
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[4][0],size,display); //��
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[5][0],size,display); //��
            segpoint += (size/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[4][0],size,display); //��
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[5][0],size,display); //��
            segpoint += (size/100); 
            #endif

            if(engerytype == CombinedOneReactivePowerEnergy)   
            {
                combinedreactivepowerenergy = 1;
            }
            if(engerytype == CombinedTwoReactivePowerEnergy) 
            {
               combinedreactivepowerenergy = 2; 
            }      
        }
        else if(engerytype == PositiveReactivePowerEnergy) //�����޹���
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[6][0],size,display); //��
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display); //��
            segpoint += (size/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[6][0],size,display); //��
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display); //��
            segpoint += (size/100); 
            #endif

        }
        else if(engerytype == ReverseReactivePowerEnergy) //�����޹���
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[7][0],size,display); //��
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display); //��
            segpoint += (size/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[7][0],size,display); //��
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display); //��
            segpoint += (size/100); 
            #endif            

        }  
        else if((engerytype == FirstQuadrantReactivePowerEnergy)||(engerytype == SecondQuadrantReactivePowerEnergy)\
              ||(engerytype == ThirdQuadrantReactivePowerEnergy)||(engerytype == FourthQuadrantReactivePowerEnergy)) //�����޹���  
        {
            if(engerytype == FirstQuadrantReactivePowerEnergy)  //��һ����
            {

                /*
                //��ʾ��I��
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[44][0],size,display); //��(44)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[44][0],size,display); //��(44)
                segpoint += (size/100);     
                #endif   
                */
                //��ʾ����1���ޡ�
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[53][0],size,display);      //��(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[1][0],sizenumber,display); //1
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //��(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //��(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[53][0],size,display);      //��(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[1][0],sizenumber,display); //1
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //��(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //��(55) 
                segpoint += (size/100);      
                #endif                  

            }
            else if(engerytype == SecondQuadrantReactivePowerEnergy)  //�ڶ�����
            {
                /*
                //��ʾ����
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[45][0],size,display); //��(45)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[45][0],size,display); //��(45)
                segpoint += (size/100);     
                #endif   
                */
                //��ʾ����2���ޡ�
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //��(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[2][0],sizenumber,display);     //2
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //��(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[56][0],size,display);      //��(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //��(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[2][0],sizenumber,display); //2
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //��(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[56][0],size,display);      //��(55) 
                segpoint += (size/100);      
                #endif 
            }
            else if(engerytype == ThirdQuadrantReactivePowerEnergy)  //��������
            {
                /*
                //��ʾ����
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Chinese_12p12p[46][0],size,display); //��(46)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[46][0],size,display); //��(46)
                segpoint += (size/100);     
                #endif 
                */
                //��ʾ����3���ޡ�
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //��(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[3][0],sizenumber,display); //3
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //��(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[56][0],size,display);      //��(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //��(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[3][0],sizenumber,display); //3
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //��(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[56][0],size,display);      //��(55) 
                segpoint += (size/100);      
                #endif                

            }
            else if(engerytype == FourthQuadrantReactivePowerEnergy)  //��������
            {
                /*
                //��ʾ������
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Chinese_12p12p[47][0],size,display); //��(47)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[47][0],size,display); //��(47)
                segpoint += (size/100);     
                #endif  
                */
               
                //��ʾ����4���ޡ�
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //��(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[4][0],sizenumber,display); //4
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //��(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[56][0],size,display);      //��(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //��(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[4][0],sizenumber,display); //4
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //��(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[56][0],size,display);      //��(55) 
                segpoint += (size/100);      
                #endif                 
            }
        } 
        
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[36][0],size,display); //��
        segpoint += (size/100); 
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[38][0],size,display);  //��
        segpoint += (size/100); 
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[36][0],size,display); //��
        segpoint += (size/100); 
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[38][0],size,display);  //��
        segpoint += (size/100);   
        #endif  


        //��ʾ����޹�1�����޹�2�е�����
        if(combinedreactivepowerenergy == 1)        
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[1][0],sizenumber,display); //1
            segpoint += (sizenumber/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[1][0],sizenumber,display); //1
            segpoint += (sizenumber/100);  
            #endif  

        }
        else if(combinedreactivepowerenergy == 2)
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[2][0],sizenumber,display); //2
            segpoint += (sizenumber/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[2][0],sizenumber,display); //2
            segpoint += (sizenumber/100);  
            #endif  
        }
    }
    else if((engerytype == PositiveApparentEnergy)||(engerytype == ReverseApparentEnergy))      //������
    {
        if(engerytype == PositiveApparentEnergy)        //��������
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[6][0],size,display);   //��
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[6][0],size,display);   //��
            #endif
            segpoint += (size/100); 
        }
        else                                            //��������
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[7][0],size,display);   //��
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[7][0],size,display);   //��
            #endif
            segpoint += (size/100); 
        }

        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display);   //��
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display);   //��
        #endif
        segpoint += (size/100); 
        
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[50][0],size,display);   //��
        segpoint += (size/100);  
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[51][0],size,display);   //��
        segpoint += (size/100);  
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[50][0],size,display);         //��
        segpoint += (size/100);  
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[51][0],size,display);        //��
        segpoint += (size/100);  
        #endif 
    }   
    return segpoint;
}

/** 
 * @brief  ���"�ܵ���"���ߡ�Tx��������������ʾ��
 * @note   
 * @param  startseg: �ַ���ʾ��ʼλ��
 * @param  rate: ���� 0~12������0�����ܣ���������Tx
 * @retval ���ص���д���seg����λ��
 */
static unsigned char Fill_EngeryRate_In_ChineseHintArea(unsigned char startseg,unsigned char rate)
{
    unsigned int size;
    unsigned int sizenumber;
    unsigned char segpoint;
    unsigned char low,high;
    
    #if (MeterType == ThreePhaseMeter)
    sizenumber = Size_6P12P;
    size = Size_12P12P;
    #else
    sizenumber = Size_7P14P;
    size = Size_14P14P;
    #endif
    segpoint =  startseg;
    
    if(rate)        //���ʵ���
    {
        //��ȡ���ʵĸߵ��ֽ�BCD��
        high=rate/10;
        low=rate%10; 

        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[10][0],sizenumber,display); //T
        segpoint += (sizenumber/100); 
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[10][0],sizenumber,display); //T
        segpoint += (sizenumber/100);  
        #endif  

        if(high != 0)
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[high][0],sizenumber,display); //X
            segpoint += (sizenumber/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[high][0],sizenumber,display); //X
            segpoint += (sizenumber/100);  
            #endif  
        }

        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[low][0],sizenumber,display); //X
        segpoint += (sizenumber/100); 
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[low][0],sizenumber,display); //X
        segpoint += (sizenumber/100);  
        #endif  

    }
    else             //�ܵ���
    {

        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[9][0],size,display); //��
        segpoint += (size/100); 

        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[9][0],size,display); //��
        segpoint += (size/100); 
        #endif 

    }    

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[23][0],size,display); //��
    segpoint += (size/100); 
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[24][0],size,display); //��
    segpoint += (size/100); 
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[23][0],size,display); //��
    segpoint += (size/100); 
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[24][0],size,display); //��
    segpoint += (size/100); 
    #endif 

    return segpoint;
}
/** 
 * @brief  ���ݵ�������ͷ����ж�Ҫ����������ʾ����
 * @note   
 * @param  phase: ��λ������μ�PHASE_TYPEö��
 * @param  engerytype: �������࣬����μ�ENERGY_TYPEö��
 * @param  date: ���ڣ���ʱ֧��0~12��0��ʾ��ǰ  ����������x��
 * @param  rate: ���� 0~12������0�����ܣ���������Tx
 * @retval None
 */
static void Fill_Engery_In_ChineseHintArea(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate)
{

    unsigned char segpoint;

    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //���LCDRAM_Buf�е�������ʾ��

    segpoint = ChineseHintAreaStartSeg;  
    //��ʾ��λ
    segpoint = Fill_Phase_In_ChineseHintArea(segpoint,phase);
    //��ʾ��ǰ����x��
    segpoint = Fill_CurrentAndLastXMonth_In_ChineseHintArea(segpoint,date);
    //��ʾ��������
    segpoint = Fill_EngeryType_In_ChineseHintArea(segpoint,engerytype);
    //��ʾ���ʵ���
    segpoint = Fill_EngeryRate_In_ChineseHintArea(segpoint,rate);
}
/** 
 * @brief  ��䡰��ǰ���ڡ���������ʾ��
 * @note   
 * @retval None
 */
static void Fill_CurrentDate_In_ChineseHintArea(void)
{
    unsigned int size;

    unsigned char segpoint;
    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //���LCDRAM_Buf�е�������ʾ��
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //ǰ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[48][0],size,display);   //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[49][0],size,display);   //��
    segpoint += (size/100);    
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //ǰ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[48][0],size,display);   //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[49][0],size,display);   //��
    segpoint += (size/100);
    #endif
}

/** 
 * @brief  ��䡰��ǰʱ�䡱��������ʾ��
 * @note   
 * @retval None
 */
static void Fill_CurrentTime_In_ChineseHintArea(void)
{
    unsigned int size;
    unsigned char segpoint;
    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //���LCDRAM_Buf�е�������ʾ��
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //ǰ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[27][0],size,display);   //ʱ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[28][0],size,display);   //��
    segpoint += (size/100);    
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //ǰ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[27][0],size,display);   //ʱ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[28][0],size,display);   //��
    segpoint += (size/100);
    #endif    
}

/** 
 * @brief  ������������䡰��ǰʣ���ѡ���������ʾ���������ǵ�����䡰��ǰʣ�����������ʾ��
 * @note   
 * @retval None
 */
static void Fill_RemainingAmount_In_ChineseHintArea(void)
{
    unsigned int size;
    unsigned char segpoint;

    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //���LCDRAM_Buf�е�������ʾ��
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //ǰ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[14][0],size,display);  //ʣ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[15][0],size,display);  //��
    segpoint += (size/100);   
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[23][0],size,display);  //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[34][0],size,display);  //��
    segpoint += (size/100);  
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //ǰ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[14][0],size,display);  //ʣ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[15][0],size,display);  //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[30][0],size,display);  //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[31][0],size,display);  //��
    segpoint += (size/100);
    #endif    
}

/** 
 * @brief  ������������䡰��ǰ͸֧��ѡ���������ʾ���������ǵ�����䡰��ǰ͸֧����������ʾ��
 * @note   
 * @retval None
 */
static void Fill_OverdraftAmount_In_ChineseHintArea(void)
{
    unsigned int size;

    unsigned char segpoint;

    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //���LCDRAM_Buf�е�������ʾ��
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //ǰ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[20][0],size,display);  //͸
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[21][0],size,display);  //֧
    segpoint += (size/100);   
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[23][0],size,display);  //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[34][0],size,display);  //��
    segpoint += (size/100);  
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //ǰ
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[20][0],size,display);  //͸
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[21][0],size,display);  //֧
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[30][0],size,display);  //��
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[31][0],size,display);  //��
    segpoint += (size/100);
    #endif    
}


/** 
 * @brief  �����ֵ�����ݵ�������
 * @note   
 * @param  valuepoint: ָ����ֵ�洢�����飬Ĭ��4�ֽ�BCD�룬����ֽڴ������λ
 * @param  decimalpoint: ������ʾ����ֵ��С��λ��0~7  
 * @param  plusminus: �����Ƿ���ʾ���ţ�    Plus������ʾ��Minus������ʾ
 * @param  displayhighzero: ��λ�Ƿ����㣬����μ�HIGHZERO_TYPEö��
 * @retval None
 */
static void Fill_Value_In_NumberArea(unsigned char* valuepoint,unsigned char decimalpoint,PLUS_MINUS plusminus,HIGHZERO_TYPE displayhighzero)
{
    unsigned char i;
    unsigned char valuebuf[6];
    unsigned char displaynumber;
    unsigned char invalidzeronumber;
    unsigned char segpoint;

    Clear_NumberArea_Of_LCDRAM_Buf();    //���LCDRAM_Buf��������

    for(i=0;i<4;i++)                     //������ֵ����
    {
        valuebuf[i] = *valuepoint;
        valuepoint++;        
    }
    
    if(displayhighzero == DisplayHighZero)                 //Ҫ���λ��0
    {
        displaynumber = 8;             //8������λ����ʾ
    }
    else                               //Ҫ���λ����0
    {
        invalidzeronumber=Get_InvalidZero_Number(valuebuf,4);     //�õ���Ч�����

        //����С��λ���жϸ�λ������λ�ã���Ϊ���뼴ʹ��λ�����㣬���Ǹ�λ���϶�Ҫ��ʾ��
        if((8-invalidzeronumber)<=(decimalpoint+1))   
        {
            displaynumber = (decimalpoint+1);
        }
        else
        {
            displaynumber = (8-invalidzeronumber);
        }
    }
    segpoint = NumberAreaEndSeg+1;        //��Ϊ�����ǿ��Ҷ���
    
    for(i=0;i<displaynumber;i++)
    {
        if((decimalpoint == i)&&(decimalpoint != 0))     //���û��С�����Ͳ���ʾС���㣬�����Ҫ����Ӧλ�ò���С����
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[0][0],Size_4P36P,display);     //��ʾС����   
        }

        segpoint -= Size_18P36P/100;
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&NumberArea_char_18p36p[((valuebuf[i/2]>>((i%2)*4))&0x0f)][0],Size_18P36P,display); //��ʾ����
    }

    if(plusminus == Minus)       //��ʾ����
    {
        segpoint -= Size_8P36P/100;   
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&MinusIcon_8p36p[0][0],Size_8P36P,display);         //��ʾ����
    }       
}

/** 
 * @brief  �������������������������
 * @note   
 * @param  datepoint: ָ�����ڴ洢�����飬Ĭ��3�ֽ�BCD�룬����ֽڴ�����
 * @retval None
 */
static void Fill_Date_In_NumberArea(unsigned char* datepoint)
{
    unsigned char i;
    unsigned char datebuf[3];
    unsigned char segpoint;

    Clear_NumberArea_Of_LCDRAM_Buf();           //���LCDRAM_Buf��������
    
    //�������ڵ�datebuf����
    for(i=0;i<3;i++)
    {
        datebuf[i] = *(datepoint+i);
    }
    
    segpoint = NumberAreaEndSeg+1;              //��Ϊ�����ǿ��Ҷ���

    for(i=0;i<6;i++)                            //��ʾ6������
    {
        if((i == 2)||(i == 4))                  //����Ӧλ�ò���С����,��Ϊ����
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[0][0],Size_4P36P,display);     //��ʾС����   
        }

        segpoint -= Size_18P36P/100;
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&NumberArea_char_18p36p[((datebuf[i/2]>>((i%2)*4))&0x0f)][0],Size_18P36P,display); //��ʾ����
    }   
}
/** 
 * @brief  ����ʱ�������������������
 * @note   
 * @param  timepoint: 
 * @retval None
 */
static void Fill_Time_In_NumberArea(unsigned char* timepoint)
{
    unsigned char i;
    unsigned char timebuf[3];
    unsigned char segpoint;

    Clear_NumberArea_Of_LCDRAM_Buf();           //���LCDRAM_Buf��������
    
    //����ʱ�䵽datebuf����
    for(i=0;i<3;i++)
    {
        timebuf[i] = *(timepoint+i);
    }
    
    segpoint = NumberAreaEndSeg+1;              //��Ϊ�����ǿ��Ҷ���

    for(i=0;i<6;i++)                            //��ʾ6������
    {
        if((i == 2)||(i == 4))                  //����Ӧλ�ò���ð��,��Ϊ����
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[1][0],Size_4P36P,display);     //��ʾð��   
        }

        segpoint -= Size_18P36P/100;
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&NumberArea_char_18p36p[((timebuf[i/2]>>((i%2)*4))&0x0f)][0],Size_18P36P,display); //��ʾ����
    }   
}

/** 
 * @brief  �����ֵ��������
 * @note   
 * @param  amountpoint:     ָ����洢�����飬Ĭ��4�ֽ�BCD�룬����ֽڴ����1,2λС��
 * @param  plusminus:       �����Ƿ���ʾ���ţ�    plus������ʾ��minus������ʾ
 * @param  displayhighzero: ��λ�Ƿ����㣬0������ʾ�� 1������ʾ
 * @retval None
 */
static void Fill_Amount_In_NumberArea(unsigned char* amountpoint,PLUS_MINUS plusminus,unsigned char displayhighzero)
{
    unsigned char i;
    unsigned char displaynumber;
    unsigned char invalidzeronumber;
    unsigned char amountbuf[3];
    unsigned char segpoint;

    Clear_NumberArea_Of_LCDRAM_Buf();           //���LCDRAM_Buf��������
    
    //����ʱ�䵽datebuf����
    for(i=0;i<4;i++)
    {
        amountbuf[i] = *(amountpoint+i);
    }
    
    segpoint = NumberAreaEndSeg+1;              //��Ϊ�����ǿ��Ҷ���

    if(displayhighzero == DisplayHighZero)      //Ҫ���λ��0
    {
        displaynumber = 8;                      //8������λ����ʾ
    }
    else                                        //Ҫ���λ����0
    {
        invalidzeronumber=Get_InvalidZero_Number(amountbuf,4);     //�õ���Ч�����

        //�̶�2λС��������������ʾ3λС��
        if(invalidzeronumber>=5)   
        {
            displaynumber = 3;
        }
        else
        {
            displaynumber = (8-invalidzeronumber);//������Ч��
        }
    }

    for(i=0;i<displaynumber;i++)                //��ʾ6������
    {
        if(i == 2)                              //����Ӧλ�ò���С����
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[0][0],Size_4P36P,display);     //��ʾС����
        }
        segpoint -= Size_18P36P/100;
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&NumberArea_char_18p36p[((amountbuf[i/2]>>((i%2)*4))&0x0f)][0],Size_18P36P,display); //��ʾ����
    } 
    if(plusminus == Minus)                      //��ʾ����
    {
        segpoint -= Size_8P36P/100;   
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&MinusIcon_8p36p[0][0],Size_8P36P,display);     //��ʾ����   
    }

}




/** 
 * @brief  ���kwh����λ��
 * @note   
 * @retval None
 */
static void  Fill_Kwh_In_UnitArea(void)
{
    unsigned char segpoint;
    unsigned int size;

    Clear_UnitArea_Of_LCDRAM_Buf();             //���LCDRAM_Buf�ĵ�λ��

    size = Size_8P12P;
    //��Ϊ�ǿ��ң�����Ҫ����д
    segpoint = UnitAreaEndSeg+1;
    segpoint -= (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_8p12p[4][0],size,display);    //h   
    segpoint -= (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_8p12p[0][0],size,display);    //W
    segpoint -= (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_8p12p[3][0],size,display);    //k

}

/** 
 * @brief  ���kvarh����λ��
 * @note   
 * @retval None
 */
static void Fill_kvah_In_UnitArea(void)
{
    unsigned char segpoint;
    unsigned int size;

    Clear_UnitArea_Of_LCDRAM_Buf();             //���LCDRAM_Buf�ĵ�λ��

    size = Size_25P12P;
    segpoint = UnitAreaEndSeg+1;
    segpoint -= (size/100);   
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_25p12p[0][0],size,display);    //kvarh
}

/** 
 * @brief  ��䡰Ԫ������λ��
 * @note   
 * @retval None
 */
static void Fill_Yuan_In_UnitArea(void)
{
    unsigned char segpoint;
    unsigned int size;

    Clear_UnitArea_Of_LCDRAM_Buf();             //���LCDRAM_Buf�ĵ�λ��

    size = Size_12P12P;
    segpoint = UnitAreaEndSeg+1;
    segpoint -= (size/100);   
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_12p12p[1][0],size,display);    //Ԫ   
}

/*����ȫ�ֺ���----------------------------------------------------------------*/
///����������ⲿ�ļ��ĺ���
/*��������ʾ����------------------------*/

/** 
 * @brief  ��ʾ��������
 * @note  
 * @param  phase: ������λ ����μ�PHASE_TYPEö�� 
 * @param  engerytype: �������࣬����μ�ENERGY_TYPEö��
 * @param  date:    ���ڣ���ʱ֧��0~12��0��ʾ��ǰ  ����������x��
 * @param  rate:    ���� 0~12������0�����ܣ���������Tx
 * @param  engerypoint: ָ������洢�����飬Ĭ��6�ֽ�BCD�룬����ֽڴ����3,4С��
 * @param  decimalpoint: ������ʾ�ĵ�����ʾ��λС����0~4
 * @param  plusminus: �����Ƿ���ʾ���ţ�    plus������ʾ��minus������ʾ
 * @param  displayhighzero: ��λ�Ƿ����㣬����μ�HIGHZERO_TYPEö��
 * @retval None
 */
extern void Display_Engery(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate,unsigned char* engerypoint,unsigned char decimalpoint,PLUS_MINUS plusminus,HIGHZERO_TYPE displayhighzero)
{
    unsigned char valuebuf[4];

    if((date>12)||(decimalpoint>4)||rate>19)    //�·ݳ���12�£�С���㳬��4λ���߷��ʳ���19��֧�֣�����
    {
        return;
    }

    //������λ�͵�������ͷ����ж�Ҫ����������ʾ����
    Fill_Engery_In_ChineseHintArea(phase,engerytype,date,rate);
    
    //���ݵ�������������β��������������
    Adjust_DecimalpointOfValue(engerypoint,valuebuf,decimalpoint);
    Fill_Value_In_NumberArea(valuebuf,decimalpoint,plusminus,displayhighzero);
    
    if((engerytype == CombinedActivePowerEnergy)||(engerytype == PositiveActivePowerEnergy)||(engerytype == ReverseActivePowerEnergy))     //�й���
    {
        //���Kwh����λ��
        Fill_Kwh_In_UnitArea();
    }
    else
    {
        //���kvah����λ��
        Fill_kvah_In_UnitArea();        
    }


    Refresh_ChineseHintArea_of_LCD_DDRAM();         //ˢ�µ�LCD��������ʾ��
    Refresh_NumberArea_of_LCD_DDRAM();              //ˢ�µ�LCD��������ʾ��
    Refresh_UnitArea_of_LCD_DDRAM();                //ˢ�µ�LCD�ĵ�λ��
}


/** 
 * @brief  ��ʾ��ǰ����
 * @note   
 * @param  datepoint: ָ�����ڴ洢�����飬Ĭ��3�ֽ�BCD�룬����ֽڴ�����
 * @retval None
 */
extern void Display_CurrentDate(unsigned char* datepoint)
{

    //��䡰��ǰ���ڡ���������ʾ����
    Fill_CurrentDate_In_ChineseHintArea();
    //�������������������������
    Fill_Date_In_NumberArea(datepoint);

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //ˢ�µ�LCD��������ʾ��
    Refresh_NumberArea_of_LCD_DDRAM();              //ˢ�µ�LCD��������ʾ��
    Refresh_UnitArea_of_LCD_DDRAM();                //ˢ�µ�LCD�ĵ�λ��(����©��ȷ����λ�����)
}

/** 
 * @brief  ��ʾ��ǰʱ��
 * @note   
 * @param  timepoint: ָ��ʱ��洢�����飬Ĭ��3�ֽ�BCD�룬����ֽڴ�����
 * @retval None
 */
extern void Display_CurrentTime(unsigned char* timepoint)
{
    //��䡰��ǰʱ�䡱��������ʾ����
    Fill_CurrentTime_In_ChineseHintArea();
    //����ʱ�������������������
    Fill_Time_In_NumberArea(timepoint);

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //ˢ�µ�LCD��������ʾ��
    Refresh_NumberArea_of_LCD_DDRAM();              //ˢ�µ�LCD��������ʾ��
    Refresh_UnitArea_of_LCD_DDRAM();                //ˢ�µ�LCD�ĵ�λ��(����©��ȷ����λ�����)
}


/** 
 * @brief  ��ʾ��ǰʣ����
 * @note   ������ʾ����ǰʣ���ѡ� ������ʾ����ǰʣ���
 * @param  amountpoint: ָ��ʣ����洢�����飬Ĭ��4�ֽ�BCD�룬����ֽڴ���С��1��2λ
 * @param  displayhighzero: ��λ�Ƿ����㣬0������ʾ�� 1������ʾ
 * @retval None
 */
extern void Display_RemainingAmount(unsigned char* amountpoint,unsigned char displayhighzero)
{
    //��䡰��ǰʣ������ߡ���ǰʣ���ѡ���������ʾ����
    Fill_RemainingAmount_In_ChineseHintArea();
    //����ʣ������������������������������
    Fill_Amount_In_NumberArea(amountpoint,Plus,displayhighzero);
    //��䡰Ԫ������λ��
    Fill_Yuan_In_UnitArea();

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //ˢ�µ�LCD��������ʾ��
    Refresh_NumberArea_of_LCD_DDRAM();              //ˢ�µ�LCD��������ʾ��
    Refresh_UnitArea_of_LCD_DDRAM();                //ˢ�µ�LCD�ĵ�λ��(����©��ȷ����λ�����)
}

/** 
 * @brief  ��ʾ��ǰ͸֧���
 * @note   ������ʾ����ǰ͸֧��ѡ� ������ʾ����ǰ͸֧��
 * @param  amountpoint: ָ��͸֧���洢�����飬Ĭ��4�ֽ�BCD�룬����ֽڴ���С��1��2λ
 * @param  displayhighzero: ��λ�Ƿ����㣬0������ʾ�� 1������ʾ
 * @retval None
 */
extern void Display_OverdraftAmount(unsigned char* amountpoint,unsigned char displayhighzero)
{
    //��䡰��ǰ͸֧�����ߡ���ǰ͸֧��ѡ���������ʾ����
    Fill_OverdraftAmount_In_ChineseHintArea();
    //����͸֧�����������������������������
    Fill_Amount_In_NumberArea(amountpoint,Minus,displayhighzero);
    //��䡰Ԫ������λ��
    Fill_Yuan_In_UnitArea();

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //ˢ�µ�LCD��������ʾ��
    Refresh_NumberArea_of_LCD_DDRAM();              //ˢ�µ�LCD��������ʾ��
    Refresh_UnitArea_of_LCD_DDRAM();                //ˢ�µ�LCD�ĵ�λ��(����©��ȷ����λ�����)    
}

/*end-------------------------------------------------------------------------*/
