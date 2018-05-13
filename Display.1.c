/*******************************************************************************
* @file      : Display.c
* @author    : Jiangsh
* @version   : V1.0.0
* @date      : Sat Apr 21 2018
* @brief     : 
********************************************************************************
* @attention :如果本文本中存在汉字变量，文本要转化成GB2312编码格式保存和打开
*
*
*/
/*头文件----------------------------------------------------------------------*/
///添加头文件
#include "Display.h"
#include "LCD.h"
#include "CharLib.h"
#include "LCDConfig.h"
#include "string.h"
/*宏定义----------------------------------------------------------------------*/
#define ChineseHintAreaGBKBufSize 40         //中文提示区GBK数组的长度

/*内部变量声明----------------------------------------------------------------*/

/* 中文提示区LCD的备份缓存数组，用于左右滚动显示 大小是中文提示区缓存数组的2倍 */
static unsigned char ChineseHintArea_LCDRAM_BackupBuf[(ChineseHintAreaEndSeg-ChineseHintAreaStartSeg)*2][ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom];
/* 中文提示区的显示内容的GBK缓存区和字符串长度 */
static short ChineseHintAreaGBKBuf[ChineseHintAreaGBKBufSize];
/* 中文提示区是否启用动态显示标志，0不启用，1是代表启用，注意，初始化时默认启用，每次填中文提示区时先清零 */
static unsigned char  ChineseHintArea_RollDisplay;

/*内部变量定义----------------------------------------------------------------*/


/*声明内部函数----------------------------------------------------------------*/
static void Clear_ChineseHintAreaGBKBuf(void);
static char Write_OneGBK_ChineseHintAreaGBKBuf(unsigned char index,short gbk);
static char Write_GBKs_ChineseHintAreaGBKStruct(unsigned char index,short* gbkcodebuf,unsigned char num);
static void Write_Str_ChineseHintAreaGBKStruct(unsigned char index,char* str);
static unsigned char Get_GBKNum_ChineseHintAreaGBKBuf(void);
static short Get_GBK_ChineseHintAreaGBKBuf(unsigned char index);


static void Clear_ChineseHintArea_LCDRAM_BackupBuf(void);
static  void ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit);
static void InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear);


const unsigned char* Get_CharBufAddress(unsigned short gbkcode);
static char ChineseHintAreaGBKBuf_Transform_ChineseHintArea_LCDRAM_BackupBuf(void);
/*定义内部函数----------------------------------------------------------------*/
//操作ChineseHintAreaGBKBuf数组的相关函数
/** 
 * @brief  清空ChineseHintAreaGBKBuf数组
 * @note   
 * @retval None
 */
static void Clear_ChineseHintAreaGBKBuf(void)
{
    unsigned char i;
    for(i=0;i<ChineseHintAreaGBKBufSize;i++)
    {
        ChineseHintAreaGBKBuf[i] = 0;
    }
}
/** 
 * @brief  写一个GBK码到ChineseHintAreaGBKBuf数组的指定位置
 * @note   
 * @param  index: 数组索引号
 * @param  gbk: gbk内容
 * @retval -1：代表越界，1代表写成功
 */
static  char Write_OneGBK_ChineseHintAreaGBKBuf(unsigned char index,short gbk)
{
    if(index>=ChineseHintAreaGBKBufSize)
    {
        return -1;
    }
    ChineseHintAreaGBKBuf[index] = gbk;
    return 1;
}
/** 
 * @brief  写一串GBK码到ChineseHintAreaGBKBuf数组中
 * @note   
 * @param  index: ChineseHintAreaGBKBuf索引号
 * @param  gbkcodebuf: gbk数组指针
 * @param  num: gbk数组长度
 * @retval -1：代表越界，1代表写成功
 */
static char Write_GBKs_ChineseHintAreaGBKBuf(unsigned char index,short* gbkcodebuf,unsigned char num)
{
    unsigned char i;
    char result;
    for(i=0;i<num;i++)
    {
     result = Write_OneGBK_ChineseHintAreaGBKBuf((index+i),*(gbkcodebuf+i));
     if(result == -1)
     {
         return result;
     }
    }
    return result;
}
/** 
 * @brief  写字符串到
 * @note   
 * @param  index: 
 * @param  str: 
 * @retval 
 */
static char Write_Str_ChineseHintAreaGBKStruct(unsigned char index,char* str)
{
    short tempgbk;
    unsigned char i;
    unsigned char len;
    unsigned char offset;
    char result;
    len = (unsigned char)strlen(str);   //得到字符串长度
    offset = 0;                         //偏移量初始化
    result = 0;                         
    for(i=0;i<len;i++)                  //根据字符串长度循环
    {
        if(*(str+offset) == 0)          //碰到回车符，结束
        {
            return result;
            
        }
        else if(*(str+offset) < 0x80)    //代表这个字符只GBK码占1个字节
        {
            tempgbk = *(str+offset);
            result = Write_OneGBKToChineseHintAreaGBK((index+i),tempgbk);
            if(result == -1)
            {
                return result;   //越界就直接返回
            }
            offset += 1;        //写完后偏移1个字节 
        }
        else if(*(str+offset) >0x80)    //代表这个字符GBK码占两字节
        {
            tempgbk = *(str+offset);
            tempgbk = tempgbk<<8+*(str+offset+1);
            result = Write_OneGBKToChineseHintAreaGBK((index+i),tempgbk);
            if(result == -1)
            {
                return result;   //越界就直接返回
            }
            offset += 2;        //写完后偏移2个字节 
        }
    }
    return result;   //写完字符串，就返回

}
/** 
 * @brief  得到ChineseHintAreaGBKBuf数组中的GBK码数量
 * @note   
 * @retval gbk码数量
 */
static unsigned char Get_GBKNum_ChineseHintAreaGBKBuf(void)
{
    unsigned char i;

    for(i=0;i<ChineseHintAreaGBKBufSize;i++)
    {
        if(ChineseHintAreaGBKBuf[i] == 0)
        {
            return i;
        }
    }
}

//操作LCDRAM_BackupBuf数组的相关函数
/** 
 * @brief  清空中文提示区备份缓存数组内容
 * @note   
 * @retval None
 */
static void Clear_ChineseHintArea_LCDRAM_BackupBuf(void)
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
/** 
 * @brief  LCDRAM_BackupBuf任意位置描点函数
 * @note   (x,y)位置的某点，显示或者不显示，
 * 注意，这个只是描了备用缓存区的点，最终显示需要调用其它函数将备用缓存区写到缓存区，再将缓存区数据写到LCD
 * @param  x: seg的位置
 * @param  y: com的位置
 * @param  bit: 0：取反 1：正常
 * @retval None
 */
static  void ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit)
{
    unsigned short pos;
    unsigned short bx;
    unsigned char temp=0;

    if((x>=(seg*2))||y>=com)return;       //超出范围
    pos=y/8;        //得到页地址       
    
    bx=y%8;         //得到点在页地址的bit位
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
 * @brief  往ChineseHintArea_LCDRAM_BackupBuf的指定位置显示或清除一个字符(包括汉字、字母、数字、符号)
 * @note   描点方式是逐列式，清除的话，确保同一位置显示了对应字符，不然不保证清除效果
 * @param  x: 指定起始位置的seg
 * @param  y: 指定起始位置的com
 * @param  charbufstartaddress: 字符的字模起始地址
 * @param  size: 字符尺寸
 * @param  displayorclear: clear代表清除，display代表显示
 * @retval None
 */
static void InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear)
{
    unsigned char temp;
    unsigned short y0;
    unsigned int t,t1,segnumber,comnumber,bytesnumber; 

    y0 = y;
    segnumber = size/100;
    comnumber = size%100;
    bytesnumber = segnumber*(comnumber/8+((comnumber%8)?1:0));      //得到需要显示所用的字节数
    
    for(t=0;t<bytesnumber;t++)                                      //将显示所用的字节全部写到LCDRAM_Buf数组中
    {     
        temp=*(charbufstartaddress+t);
        for(t1=0;t1<8;t1++) 
        {
            if(temp&0x80)
            {
 
                if((x>=(seg*2))||(y>=com))  //超范围了
                {
                    return;                 //超出数组范围的，不写
                }
                if(displayorclear == display)      //写1，即显示
                {
                    ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(x,y,1);
                }
                else                                //写0，即清除
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
/** 
 * @brief  根据GBK码返回字符字库内容所在位置
 * @note   
 * @param  gbkcode: gbk码
 * @retval 所在位置的指针
 */
const unsigned char* Get_CharBufAddress(short gbkcode)
{   
    unsigned char i;

    for(i=0;i<GBKNumber;i++)
    {
        if(gbkcode == charaddressbuf[i].CharGbk)
        {
            return charaddressbuf[i].CharAddress;
        }
    }
    //找不到，就返回数字0的位置
    #if (MeterType == ThreePhaseMeter)
    return &ChineseHint_Char_6p12p[0][0];
    #else
    return &ChineseHint_Char_7p14p[0][0];    
    #endif
}

/** 
 * @brief  将ChineseHintAreaGBKBuf数组中的GBK码转化成点阵信息到ChineseHintArea_LCDRAM_BackupBuf
 * @note   
 * @retval -1超出ChineseHintArea_LCDRAM_BackupBuf范围，越界 1写成功
 */
static char ChineseHintAreaGBKBuf_Transform_ChineseHintArea_LCDRAM_BackupBuf(void)
{
    unsigned char len;
    unsigned int sizenumber;
    unsigned int size;
    const unsigned char* address;
    short tempgbk;
    unsigned char short offset;

    offset = 0;
    Clear_ChineseHintArea_LCDRAM_BackupBuf();   //清空
    len = Get_GBKNum_ChineseHintAreaGBKBuf();   //得到GBK码数量

    #if (MeterType == ThreePhaseMeter)          //根据表型，确定尺寸
    sizenumber = Size_6P12P;
    size = Size_12P12P;
    #else
    sizenumber = Size_7P14P;
    size = Size_14P14P;
    #endif

    for(i=0;i<len;i++)
    {   
        tempgbk = Get_GBK_ChineseHintAreaGBKBuf(i);
        address = Get_CharBufAddress(tempgbk);

        if(offset>=((ChineseHintAreaEndSeg-ChineseHintAreaStartSeg)*2))
        {
            return -1;
        }
        if(tempgbk > 0x8000)    //汉字
        {
            InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(offset,0,address,size,display);
            offset += size/100;
        }
        else                    //数字字母
        {
            InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(offset,0,address,sizenumber,display);
            offset += sizenumber/100;
        }
    }

    return 1;
}
/** 
 * @brief  将ChineseHintArea_LCDRAM_BackupBuf拷贝到LCDRAM_Buf
 * @note   ChineseHintArea_LCDRAM_BackupBuf的起始位置+offset开始拷贝到LCDRAM_Buf[ChineseHintAreaStartSeg~ChineseHintAreaStartSeg+len]
 * @param  len: 拷贝的长度
 * @param  offset: 偏移量
 * @retval None
 */
static void Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf(unsigned char len,unsigned short offset)
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

static void Fill_Char_In_ChineseHintArea(unsigned char* strbuf,char SplitScreen)
{
        unsigned char i;
    const unsigned char* addresspoint;
    unsigned short segpoint;
    unsigned char segendpoint;
    unsigned int sizenumber;
    unsigned int size;
    
    //初始化
    Clear_ChineseHintAreaGBKBuf();

    ChineseHintArea_RollDisplay = 0;                //滚动显示标志位清零
    ChineseHintAreaGBKLen = 0;                      //有效GBK码数量清零
    Clear_ChineseHintArea_Of_LCDRAM_Buf();          //字模内容缓存区清零
    Clear_ChineseHintArea_LCDRAM_BackupBuf();      //字模内容备份缓存区清零

    //判断分屏是否显示
    switch(SplitScreen)
    {
        case NoDisplaySplitScreen :
        
        break;
            
        case DisplaySplitScreenZero :
            #if (MeterType == ThreePhaseMeter)
            addresspoint = &SplitScreenIcon_13p12p[0][0];
            size = Size_13P12P;
            #else
            addresspoint = &SplitScreenIcon_14p14p[0][0];
            size = Size_14P14P;
            #endif
        break;

        case DisplaySplitScreenOne :
            #if (MeterType == ThreePhaseMeter)
            addresspoint = &SplitScreenIcon_13p12p[1][0];
            size = Size_13P12P;
            #else
            addresspoint = &SplitScreenIcon_14p14p[1][0];
            size = Size_14P14P;
            #endif
        break;

        case DisplaySplitScreenTwo :
            #if (MeterType == ThreePhaseMeter)
            addresspoint = &SplitScreenIcon_13p12p[2][0];
            size = Size_13P12P;
            #else
            addresspoint = &SplitScreenIcon_14p14p[2][0];
            size = Size_14P14P;
            #endif
        break;

        case DisplaySplitScreenThree :
            #if (MeterType == ThreePhaseMeter)
            addresspoint = &SplitScreenIcon_13p12p[3][0];
            size = Size_13P12P;
            #else
            addresspoint = &SplitScreenIcon_14p14p[3][0];
            size = Size_14P14P;
            #endif
        break;

        default:
        break;
    }
    if(SplitScreen != NoDisplaySplitScreen)
    {
        /* 显示分屏 */
        InputCharacter_to_LCDRAM_Buf(SplitWindowAreaStartseg,SplitWindowAreaStartCom,addresspoint,size,display);

        segendpoint = SplitWindowAreaStartseg;      //确定中文提示区文字内容的结束seg地址
    }
    else
    {
        segendpoint = ChineseHintAreaEndSeg;        //确定中文提示区文字内容的结束seg地址
    }

    //确定中文提示区文字内容
    #if (MeterType == ThreePhaseMeter)
    sizenumber = Size_6P12P;
    size = Size_12P12P;
    #else
    sizenumber = Size_7P14P;
    size = Size_14P14P;
    #endif
}

static void Fill_Engery_In_ChineseHintArea(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate)
{
    unsigned char index;
    short tempgbk;

    if((date>99)||rate>99)    //月份超过99月，小数点超过4位或者费率超过99不支持，返回
    {
        return;
    }

    Clear_ChineseHintAreaGBKBuf();

    index = 0;
    if(date == 0)       //当前
    {
        str = "当前";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }
    else if(date < 10)  //上x月
    {
        str = "上";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
        tempgbk = 0x0030+(short)date;
        Write_OneGBK_ChineseHintAreaGBKBuf(index,tempgbk);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }
    else if(date > 10)  //上xx月
    {
        str = "上";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
        tempgbk = 0x0030+(short)(date/10);
        Write_OneGBK_ChineseHintAreaGBKBuf(index,tempgbk);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
        tempgbk = 0x0030+(short)(date%10);
        Write_OneGBK_ChineseHintAreaGBKBuf(index,tempgbk);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }

    if(phase == APhase)
    {
        str = "A相";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }
    else if(phase == BPhase)
    {
        str = "B相";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }
    else if(phase == CPhase)
    {
        str = "C相";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }

    if(engerytype == CombinedActivePowerEnergy)
    {
        #if (MeterType == ThreePhaseMeter)          //根据表型，确定尺寸
        str = "组合有功";
        #else
        str = "有功";
        #endif
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }
    else if(engerytype == PositiveActivePowerEnergy)
    {
        str = "正向有功";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }
    else if(engerytype == ReverseActivePowerEnergy)
    {
        str = "反向有功电量";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }
    else if(engerytype == CombinedOneReactivePowerEnergy)
    {
        str = "组合无功1";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }
    else if(engerytype == CombinedTwoReactivePowerEnergy)
    {
        str = "组合无功2";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }
    else if(engerytype == PositiveReactivePowerEnergy)
    {
        str = "正向无功";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }    
    else if(engerytype == ReverseReactivePowerEnergy)
    {
        str = "反向无功";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }    
    else if(engerytype == FirstQuadrantReactivePowerEnergy)
    {
        str = "第1象限无功";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }    
    else if(engerytype == SecondQuadrantReactivePowerEnergy)
    {
        str = "第2象限无功";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }    
    else if(engerytype == ThirdQuadrantReactivePowerEnergy)
    {
        str = "第3象限无功";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }    
    else if(engerytype == FourthQuadrantReactivePowerEnergy)
    {
        str = "第4象限无功";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }    
    else if(engerytype == PositiveApparentEnergy)
    {
        str = "正向视在";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }    
    else if(engerytype == FourthQuadrantReactivePowerEnergy)
    {
        str = "反向视在";
        Write_Str_ChineseHintAreaGBKStruct(index,str);
        index = Get_GBKNum_ChineseHintAreaGBKBuf();
    }    

    str = "电量";
    Write_Str_ChineseHintAreaGBKStruct(index,str);
    index = Get_GBKNum_ChineseHintAreaGBKBuf();

    ChineseHintAreaGBKBuf_Transform_ChineseHintArea_LCDRAM_BackupBuf();

    Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf((ChineseHintAreaEndSeg-ChineseHintAreaStartSeg),0);

}




/*定义全局函数----------------------------------------------------------------*/
///定义可用于外部文件的函数
/*电表相关显示函数------------------------*/

/** 
 * @brief  显示电量函数
 * @note  
 * @param  phase: 代表相位 具体参见PHASE_TYPE枚举 
 * @param  engerytype: 电量种类，具体参见ENERGY_TYPE枚举
 * @param  date:    日期，暂时支持0~99，0表示当前  其他代表上x月
 * @param  rate:    费率 0~99，其中0代表总，其他代表Tx
 * @param  engerypoint: 指向电量存储的数组，默认6字节BCD码，最低字节代表第3,4小数
 * @param  decimalpoint: 代表显示的电量显示几位小数，0~4
 * @param  plusminus: 代表是否显示负号，    plus代表不显示，minus代表显示
 * @param  displayhighzero: 高位是否显零，具体参见HIGHZERO_TYPE枚举
 * @retval None
 */
extern void Display_Engery(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate,unsigned char* engerypoint,unsigned char decimalpoint,PLUS_MINUS plusminus,HIGHZERO_TYPE displayhighzero)
{
    unsigned char valuebuf[4];

    if((date>99)||(decimalpoint>4)||rate>99)    //月份超过99月，小数点超过4位或者费率超过99不支持，返回
    {
        return;
    }

    //根据相位和电量种类和费率判断要填充的中文提示内容
    Fill_Engery_In_ChineseHintArea(phase,engerytype,date,rate);
    
    //根据电量数组和其它形参填充数字区内容
    Adjust_DecimalpointOfValue(engerypoint,valuebuf,decimalpoint);
    Fill_Value_In_NumberArea(valuebuf,decimalpoint,plusminus,displayhighzero);
    
    if((engerytype == CombinedActivePowerEnergy)||(engerytype == PositiveActivePowerEnergy)||(engerytype == ReverseActivePowerEnergy))     //有功类
    {
        //填充Kwh到单位区
        Fill_Kwh_In_UnitArea();
    }
    else
    {
        //填充kvah到单位区
        Fill_kvah_In_UnitArea();        
    }


    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
    Refresh_NumberArea_of_LCD_DDRAM();              //刷新到LCD的数字提示区
    Refresh_UnitArea_of_LCD_DDRAM();                //刷新到LCD的单位区
}
/*end-------------------------------------------------------------------------*/
