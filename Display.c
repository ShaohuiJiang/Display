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

/*宏定义----------------------------------------------------------------------*/
///添加宏定义

typedef struct
{
    short GBKBuf[ChineseHintAreaGBKBufSize];        //字符串GBK码缓存区
    int len;                              //字符串长度
}GBKBUF_LENGTH_TYPE;


#define ChineseHintAreaGBKBufSize 20    //中文提示区字符GBK码缓存区的大小
/*内部变量声明----------------------------------------------------------------*/
///添加内部变量
static unsigned short ChineseHintAreaGBKBuf[ChineseHintAreaGBKBufSize];  //中文提示区字符GBK码缓存区
static unsigned char  ChineseHintAreaGBKLen;                             //中文提示区有效GBK码数量
/* 中文提示区LCD的备份缓存数组，用于左右滚动显示 大小是中文提示区缓存数组的2倍 */
static unsigned char ChineseHintArea_LCDRAM_BackupBuf[(ChineseHintAreaEndSeg-ChineseHintAreaStartSeg)*2][ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom];
static GBKBUF_LENGTH_TYPE ChineseHintAreaGBKStruct;
static unsigned char  ChineseHintArea_RollDisplay;


/*内部变量定义----------------------------------------------------------------*/



/*声明内部函数----------------------------------------------------------------*/
static unsigned char Get_InvalidZero_Number(unsigned char* buf,unsigned char len);
static void  Adjust_DecimalpointOfValue(unsigned char* srcbuf,unsigned char* objbuf,unsigned char decimalpoint);
const unsigned char* Get_CharBufAddress(unsigned short gbkcode);
static unsigned char  Get_GBKCodeOfStr(unsigned char* strpoint,unsigned short* gbkbufpoint);

static void Clear_ChineseHintArea_LCDRAM_BackupBuf(void);
static  void ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit);
static void InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear);
static void Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf(unsigned char len,unsigned short offset);
static void Fill_Char_In_ChineseHintArea(unsigned char* strbuf,SPLITSCREENDISPLAY_TYPE SplitScreen);

/*定义内部函数----------------------------------------------------------------*/
/** 
 * @brief  获取数组中高位无效0的个数
 * @note   数组必须为BCD格式，并默认低字节数组存放低位数据
 * @param  buf: 被判断的数组起始地址
 * @param  len: 被判断的数据字节长度  0~10
 * @retval 无效0个数
 */
static unsigned char Get_InvalidZero_Number(unsigned char* buf,unsigned char len)
{
    unsigned char temp;
    unsigned char i;
    unsigned char number;

    if(len>10)          //超长了，返回
    {
        return 255;
    }

    number = 0;         //初始化

    for(i=0;i<len;i++)          //拷贝最高位字节数据并判断是否为0，并循环
    {
        temp = *(buf+len-1-i); 
        if((temp&0xf0) == 0x00)   //先判断高位：等于0，就无效0数加1，否则就直接返回
        {
        number++;
        }
        else                    //碰到非零，结束判断，并返回
        {
            return number;
        }            
        if((temp&0x0f) == 0x00)   //再判断低位：等于0，就无效0数加1，否则就直接返回
        {
            number++;
        }
        else                    //碰到非零，结束判断，并返回
        {
            return number;
        }

    }
    
    return number;              //循环完了，也返回
}
/** 
 * @brief  调整4位小数的6字节数组保留的小数位复制到目标数组
 * @note   注意源数组的格式，必须满足6字节，并默认最低字节2个数组元素为小数
 *         该函数只要是为了处理4位小数的电量
 * @param  srcbuf: 6字节数组，最低字节2个数组元素为小数，BCD格式
 * @param  objbuf: 4字节数组，BCD格式
 * @param  decimalpoint: 准备保留的小数个数，0~4
 * @retval None
 */
static void  Adjust_DecimalpointOfValue(unsigned char* srcbuf,unsigned char* objbuf,unsigned char decimalpoint)
{
    
    if(decimalpoint>4)  //超范围了，直接返回
    {
        return;
    }

    switch(decimalpoint)//根据小数位调整目标数组
    {
        case 0:             //无小数位
        {
            *(objbuf)   = *(srcbuf+2);
            *(objbuf+1) = *(srcbuf+3);
            *(objbuf+2) = *(srcbuf+4);
            *(objbuf+3) = *(srcbuf+5);
        }
        break;
        case 1:             //1位小数位
        {
            *(objbuf)   = ((*(srcbuf+1)>>4)&0x0f)|((*(srcbuf+2)<<4)&0xf0);
            *(objbuf+1) = ((*(srcbuf+2)>>4)&0x0f)|((*(srcbuf+3)<<4)&0xf0);
            *(objbuf+2) = ((*(srcbuf+3)>>4)&0x0f)|((*(srcbuf+4)<<4)&0xf0);
            *(objbuf+3) = ((*(srcbuf+4)>>4)&0x0f)|((*(srcbuf+5)<<4)&0xf0);   
        }
        break;

        case 2:             //2位小数位
        {

            *(objbuf)   = *(srcbuf+1);
            *(objbuf+1) = *(srcbuf+2);
            *(objbuf+2) = *(srcbuf+3);
            *(objbuf+3) = *(srcbuf+4);
        }
        break;

        case 3:             //3位小数位
        {
            *(objbuf)   = ((*(srcbuf+0)>>4)&0x0f)|((*(srcbuf+1)<<4)&0xf0);
            *(objbuf+1) = ((*(srcbuf+1)>>4)&0x0f)|((*(srcbuf+2)<<4)&0xf0);
            *(objbuf+2) = ((*(srcbuf+2)>>4)&0x0f)|((*(srcbuf+3)<<4)&0xf0);
            *(objbuf+3) = ((*(srcbuf+3)>>4)&0x0f)|((*(srcbuf+4)<<4)&0xf0);

        }
        break;
        case 4:             //4位小数位
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
 * @brief  根据GBK码返回字符字库内容所在位置
 * @note   
 * @param  gbkcode: gbk码
 * @retval 所在位置的指针
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
    //找不到，就返回数字0的位置
    #if (MeterType == ThreePhaseMeter)
    return &ChineseHint_Char_6p12p[0][0];
    #else
    return &ChineseHint_Char_7p14p[0][0];    
    #endif
}
/** 
 * @brief   将字符串转化成GBK码
 * @note   
 * @param  strpoint: 字符串指针
 * @param  gbkbufpoint: 转化成的GBK码存放数组的地址指针
 * @retval GBK码数量
 */
static unsigned char  Get_GBKCodeOfStr(unsigned char* strpoint,unsigned short* gbkbufpoint)
{
    unsigned char i;
    unsigned char charnumberpoint=0;
    
    for(i=0;i<42;i++)           //赋值字符串的GBK代码解析并转存，同时反馈长度
    {
        if(*(strpoint+i) == 0)  //碰到\n 代表已经结束
        {
           return charnumberpoint;              //退出
        }
        else if(((*(strpoint+i))&0x80) == 0x80)      //最高位为1，代表是汉字编码一部分
        {
            i++;                            
            if(((*(strpoint+i))&0x80) == 0x80)      //最高位为1，代表是汉字编码一部分
            {
               *gbkbufpoint =  (*(strpoint+i-1))*256+(*(strpoint+i));       //得到2字节的GBK码
            }
            else                           //前一个是汉字编码一部分，后一个不是，不符合规律，不解析
            {
                continue;                  //停止这一次，继续下一次循环
            }            
        }
        else                              //最高位不是1，且不是\n 就代表是字母或者数字
        {
            *gbkbufpoint =  *(strpoint+i); //得到2字节的GBK码，高位字节为00
        }        
        //能执行到这里，说明一个字符解析成功
        gbkbufpoint++;                      //GBK编码数组指针加1
        charnumberpoint++;                  //有效字符数加1 
    }

    return charnumberpoint;              //退出

}

/*操作LCDRAM_BackupBuf数组函数------------------------*/
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
/** 
 * @brief  填充内容到中文提示区的缓存数组和备用数组
 * @note   这里先写到备用缓存数组然后判断是否超过缓存数组大小，假如超过，就启用动态显示
 * @param  strbuf: 字符串变量起始地址
 * @param  SplitScreen: 分屏是否显示或者显示几号分屏
 * @retval None
 */
static void Fill_Char_In_ChineseHintArea(unsigned char* strbuf,SPLITSCREENDISPLAY_TYPE SplitScreen)
{
    unsigned char i;
    const unsigned char* addresspoint;
    unsigned short segpoint;
    unsigned char segendpoint;
    unsigned int sizenumber;
    unsigned int size;
    
    //初始化
    for(i=0;i<ChineseHintAreaGBKBufSize;i++)
    {
        ChineseHintAreaGBKBuf[i] = 0;   //GBK码缓存区清零
    }

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

    ChineseHintAreaGBKLen = Get_GBKCodeOfStr(strbuf,ChineseHintAreaGBKBuf);    //得到有效GBK码和数量

    if(ChineseHintAreaGBKLen>ChineseHintAreaGBKBufSize) //先判断有效GBK码数量，默认支持ChineseHintAreaGBKBufSize个GBK码，如果超过，有可能Clear_LCDRAM_BackupBuf不够用，就要直接返回，不继续执行
    {
        return;
    }
    else            //没有超过，就将内容写到备用缓存数组中
    {
        segpoint = 0;       //备用缓存数组是从起始地址开始写
        
        for(i=0;i<ChineseHintAreaGBKLen;i++)
        {
            addresspoint = Get_CharBufAddress(ChineseHintAreaGBKBuf[i]);    //得到字符所在地址

            if(segpoint>(2*seg))        //超过备用缓存区数组，就返回
            {
                break;
            }

            if((ChineseHintAreaGBKBuf[i]&0x8080) == 0x8080 )    //汉字
            {
               InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(segpoint,ChineseHintAreaStartCom,addresspoint,size,display);  
               segpoint += size/100;
            }
            else                                                //字母和数字
            {
               InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(segpoint,ChineseHintAreaStartCom,addresspoint,sizenumber,display);   
               segpoint += sizenumber/100;    
            }
        }  

        //将GBK全部转化成字模内容后，就要判断全部内容有没有超中文提示区范围，假如超了就要启用滚动显示
        Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf((segendpoint-ChineseHintAreaStartSeg),0);
        if(segpoint>=segendpoint)  //超了
        {
            ChineseHintArea_RollDisplay =1;     //启用滚动显示
        }
    }
}
/** 
 * @brief  填充“当前组合有功总电量”或者“当前有功总电量”到中文提示区
 * @note   不显示分屏
 * @retval None
 */
static void Fill_CurrentCombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "当前组合有功总电量";
    #else
    str = "当前有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}
/** 
* @brief  填充“上1月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last1CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上1月组合有功总电量";
    #else
    str = "上1月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     


/** 
* @brief  填充“上2月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last2CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上2月组合有功总电量";
    #else
    str = "上2月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     


/** 
* @brief  填充“上3月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last3CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上3月组合有功总电量";
    #else
    str = "上3月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     


/** 
* @brief  填充“上4月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last4CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上4月组合有功总电量";
    #else
    str = "上4月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     
/** 
* @brief  填充“上5月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last5CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上5月组合有功总电量";
    #else
    str = "上5月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     
/** 
* @brief  填充“上6月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last6CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上6月组合有功总电量";
    #else
    str = "上6月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     
/** 
* @brief  填充“上7月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last7CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上7月组合有功总电量";
    #else
    str = "上7月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     
/** 
* @brief  填充“上8月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last8CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上8月组合有功总电量";
    #else
    str = "上8月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     
/** 
* @brief  填充“上9月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last9CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上9月组合有功总电量";
    #else
    str = "上9月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     
/** 
* @brief  填充“上10月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last10CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上10月组合有功总电量";
    #else
    str = "上10月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     
/** 
* @brief  填充“上11月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last11CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上11月组合有功总电量";
    #else
    str = "上11月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}     
/** 
* @brief  填充“上12月组合有功总电量”到中文提示区
* @note   单相不显示“组合”
* @retval None
*/
static void Fill_Last12CombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "上12月组合有功总电量";
    #else
    str = "上12月有功总电量";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}


/** 
 * @brief  填充“当前”或者“上x月”到中文提示区
 * @note   
 * @param  startseg: 显示的起始位置
 * @param  date: 日期，暂时支持0~12，0表示当前  其他代表上x月
 * @retval 返回的是写完后seg所在位置
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
    if(date == 0)           //显示当前       
    {
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //当
        segpoint += (size/100);
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //前
        segpoint += (size/100);
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //当
        segpoint += (size/100);
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //前
        segpoint += (size/100);
        #endif
    }
    else                   //显示上X月
    {
        //获取月份的高低字节BCD码
        high=date/10;
        low=date%10;
       
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[2][0],size,display);   //上
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[2][0],size,display);   //上
        #endif
        segpoint += (size/100);

        //显示XX
        if(high != 0)       //高位不为0，就需要显示
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
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[3][0],size,display);   //月
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[3][0],size,display);   //月
        #endif
        segpoint += (size/100);
    }

    return segpoint;
}

/** 
 * @brief  填充“A相”“B相”“C相”到中文提示区
 * @note   合相不显示
 * @param  startseg: 显示的起始位置
 * @param  phase: 相位类型，具体参见PHASE_TYPE枚举
 * @retval 返回的是写完后seg所在位置
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
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[52][0],size,display);   //相
        segpoint += (size/100);
    }
    #endif

    return segpoint;
}
/** 
 * @brief  填充“组合有功”“正向有功”“反向有功”“无功组合1”“无功组合2”“正向无功”“反向无功”“无功Ⅰ”“无功Ⅱ”“无功Ⅲ”“无功Ⅳ”“正向视在”“反向视在”到中文提示区
 * @note   
 * @param  startseg: 字符显示起始位置
 * @param  engerytype: 电量种类，具体参见ENERGY_TYPE枚举
 * @retval 返回的是写完后seg所在位置
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

    if((engerytype == CombinedActivePowerEnergy)||(engerytype == PositiveActivePowerEnergy)||(engerytype == ReverseActivePowerEnergy))     //有功类
    {

        if(engerytype == PositiveActivePowerEnergy)      //正向有功类
        {           
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[6][0],size,display);   //正
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[6][0],size,display);   //正
            #endif
            segpoint += (size/100);             
        }
        else if(engerytype == ReverseActivePowerEnergy) //反向有功类   
        {  
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[7][0],size,display);   //反
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[7][0],size,display);   //反
            #endif
            segpoint += (size/100);             
        }
        
        if(engerytype != CombinedActivePowerEnergy) //不是组合，就显示“向”
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display);   //向
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display);     //向
            #endif
            segpoint += (size/100);  
        }

        #if (MeterType == ThreePhaseMeter)          //单相表不需要显示有功
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[37][0],size,display); //有
        segpoint += (size/100); 
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[38][0],size,display);//功
        segpoint += (size/100);   
        #endif   
    }
    else if((engerytype == CombinedOneReactivePowerEnergy)||(engerytype == CombinedTwoReactivePowerEnergy)\
          ||(engerytype == PositiveReactivePowerEnergy)||(engerytype == ReverseReactivePowerEnergy)\
          ||(engerytype == FirstQuadrantReactivePowerEnergy)||(engerytype == SecondQuadrantReactivePowerEnergy)\
          ||(engerytype == ThirdQuadrantReactivePowerEnergy)||(engerytype == FourthQuadrantReactivePowerEnergy))     //无功类
    {
        combinedreactivepowerenergy = 0;

        if((engerytype == CombinedOneReactivePowerEnergy)||(engerytype == CombinedTwoReactivePowerEnergy))      //组合无功类
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[4][0],size,display); //组
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[5][0],size,display); //合
            segpoint += (size/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[4][0],size,display); //组
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[5][0],size,display); //合
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
        else if(engerytype == PositiveReactivePowerEnergy) //正向无功类
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[6][0],size,display); //正
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display); //向
            segpoint += (size/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[6][0],size,display); //正
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display); //向
            segpoint += (size/100); 
            #endif

        }
        else if(engerytype == ReverseReactivePowerEnergy) //反向无功类
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[7][0],size,display); //反
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display); //向
            segpoint += (size/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[7][0],size,display); //反
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display); //向
            segpoint += (size/100); 
            #endif            

        }  
        else if((engerytype == FirstQuadrantReactivePowerEnergy)||(engerytype == SecondQuadrantReactivePowerEnergy)\
              ||(engerytype == ThirdQuadrantReactivePowerEnergy)||(engerytype == FourthQuadrantReactivePowerEnergy)) //象限无功类  
        {
            if(engerytype == FirstQuadrantReactivePowerEnergy)  //第一象限
            {

                /*
                //显示“I”
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[44][0],size,display); //Ⅰ(44)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[44][0],size,display); //Ⅰ(44)
                segpoint += (size/100);     
                #endif   
                */
                //显示“第1象限”
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[53][0],size,display);      //第(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[1][0],sizenumber,display); //1
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //象(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //限(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[53][0],size,display);      //第(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[1][0],sizenumber,display); //1
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //象(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //限(55) 
                segpoint += (size/100);      
                #endif                  

            }
            else if(engerytype == SecondQuadrantReactivePowerEnergy)  //第二象限
            {
                /*
                //显示“Ⅱ”
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[45][0],size,display); //Ⅱ(45)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[45][0],size,display); //Ⅱ(45)
                segpoint += (size/100);     
                #endif   
                */
                //显示“第2象限”
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //第(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[2][0],sizenumber,display);     //2
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //象(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[56][0],size,display);      //限(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //第(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[2][0],sizenumber,display); //2
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //象(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[56][0],size,display);      //限(55) 
                segpoint += (size/100);      
                #endif 
            }
            else if(engerytype == ThirdQuadrantReactivePowerEnergy)  //第三象限
            {
                /*
                //显示“Ⅲ”
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Chinese_12p12p[46][0],size,display); //Ⅲ(46)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[46][0],size,display); //Ⅲ(46)
                segpoint += (size/100);     
                #endif 
                */
                //显示“第3象限”
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //第(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[3][0],sizenumber,display); //3
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //象(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[56][0],size,display);      //限(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //第(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[3][0],sizenumber,display); //3
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //象(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[56][0],size,display);      //限(55) 
                segpoint += (size/100);      
                #endif                

            }
            else if(engerytype == FourthQuadrantReactivePowerEnergy)  //第四象限
            {
                /*
                //显示“Ⅳ”
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Chinese_12p12p[47][0],size,display); //Ⅳ(47)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[47][0],size,display); //Ⅳ(47)
                segpoint += (size/100);     
                #endif  
                */
               
                //显示“第4象限”
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //第(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[4][0],sizenumber,display); //4
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //象(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[56][0],size,display);      //限(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //第(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[4][0],sizenumber,display); //4
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //象(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[56][0],size,display);      //限(55) 
                segpoint += (size/100);      
                #endif                 
            }
        } 
        
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[36][0],size,display); //无
        segpoint += (size/100); 
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[38][0],size,display);  //功
        segpoint += (size/100); 
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[36][0],size,display); //无
        segpoint += (size/100); 
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[38][0],size,display);  //功
        segpoint += (size/100);   
        #endif  


        //显示组合无功1或者无功2中的数字
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
    else if((engerytype == PositiveApparentEnergy)||(engerytype == ReverseApparentEnergy))      //视在类
    {
        if(engerytype == PositiveApparentEnergy)        //正向视在
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[6][0],size,display);   //正
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[6][0],size,display);   //正
            #endif
            segpoint += (size/100); 
        }
        else                                            //反向视在
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[7][0],size,display);   //反
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[7][0],size,display);   //反
            #endif
            segpoint += (size/100); 
        }

        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display);   //向
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display);   //向
        #endif
        segpoint += (size/100); 
        
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[50][0],size,display);   //视
        segpoint += (size/100);  
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[51][0],size,display);   //在
        segpoint += (size/100);  
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[50][0],size,display);         //视
        segpoint += (size/100);  
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[51][0],size,display);        //在
        segpoint += (size/100);  
        #endif 
    }   
    return segpoint;
}

/** 
 * @brief  填充"总电量"或者“Tx电量”到中文提示区
 * @note   
 * @param  startseg: 字符显示起始位置
 * @param  rate: 费率 0~12，其中0代表总，其他代表Tx
 * @retval 返回的是写完后seg所在位置
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
    
    if(rate)        //费率电量
    {
        //获取费率的高低字节BCD码
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
    else             //总电量
    {

        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[9][0],size,display); //总
        segpoint += (size/100); 

        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[9][0],size,display); //总
        segpoint += (size/100); 
        #endif 

    }    

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[23][0],size,display); //电
    segpoint += (size/100); 
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[24][0],size,display); //量
    segpoint += (size/100); 
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[23][0],size,display); //电
    segpoint += (size/100); 
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[24][0],size,display); //量
    segpoint += (size/100); 
    #endif 

    return segpoint;
}
/** 
 * @brief  根据电量种类和费率判断要填充的中文提示内容
 * @note   
 * @param  phase: 相位，具体参见PHASE_TYPE枚举
 * @param  engerytype: 电量种类，具体参见ENERGY_TYPE枚举
 * @param  date: 日期，暂时支持0~12，0表示当前  其他代表上x月
 * @param  rate: 费率 0~12，其中0代表总，其他代表Tx
 * @retval None
 */
static void Fill_Engery_In_ChineseHintArea(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate)
{
    unsigned char i;
    unsigned char segpoint;
    char* str;
    Clear_ChineseHintArea_Of_LCDRAM_Buf();      //清空LCDRAM_Buf中的中文提示区
    
    for(i=0;i<ChineseHintAreaGBKBufSize;i++)    //清空GBK码数组缓存区
    {
        ChineseHintAreaGBKStruct.GBKBuf[i] = 0;
    }
    ChineseHintAreaGBKStruct.len = 0;           //字符串长度清零
    switch(phase)
    {   
        case APhase:
            str = "A相";
        break;
        
        case BPhase:
            str = "B相";
        break;
        
        case CPhase:
            str = "C相";
        break;

        default:
        break;
    }

    if(phase != TotalPhase)
    {
         ChineseHintAreaGBKStruct.len +=strlen(str);    //得到长度
         for(i=0;i<(unsigned char)strlen(str);i++)
    }




    segpoint = ChineseHintAreaStartSeg;  
    //显示相位
    segpoint = Fill_Phase_In_ChineseHintArea(segpoint,phase);
    //显示当前或上x月
    segpoint = Fill_CurrentAndLastXMonth_In_ChineseHintArea(segpoint,date);
    //显示电量种类
    segpoint = Fill_EngeryType_In_ChineseHintArea(segpoint,engerytype);
    //显示费率电量
    segpoint = Fill_EngeryRate_In_ChineseHintArea(segpoint,rate);
}
/** 
 * @brief  填充“当前日期”到中文提示区
 * @note   
 * @retval None
 */
static void Fill_CurrentDate_In_ChineseHintArea(void)
{
    unsigned int size;

    unsigned char segpoint;
    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //清空LCDRAM_Buf中的中文提示区
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //当
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //前
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[48][0],size,display);   //日
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[49][0],size,display);   //期
    segpoint += (size/100);    
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //当
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //前
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[48][0],size,display);   //日
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[49][0],size,display);   //期
    segpoint += (size/100);
    #endif
}

/** 
 * @brief  填充“当前时间”到中文提示区
 * @note   
 * @retval None
 */
static void Fill_CurrentTime_In_ChineseHintArea(void)
{
    unsigned int size;
    unsigned char segpoint;
    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //清空LCDRAM_Buf中的中文提示区
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //当
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //前
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[27][0],size,display);   //时
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[28][0],size,display);   //间
    segpoint += (size/100);    
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //当
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //前
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[27][0],size,display);   //时
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[28][0],size,display);   //间
    segpoint += (size/100);
    #endif    
}

/** 
 * @brief  假如是三相填充“当前剩余电费”到中文提示区，假如是单相填充“当前剩余金额”到中文提示区
 * @note   
 * @retval None
 */
static void Fill_RemainingAmount_In_ChineseHintArea(void)
{
    unsigned int size;
    unsigned char segpoint;

    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //清空LCDRAM_Buf中的中文提示区
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //当
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //前
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[14][0],size,display);  //剩
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[15][0],size,display);  //余
    segpoint += (size/100);   
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[23][0],size,display);  //电
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[34][0],size,display);  //费
    segpoint += (size/100);  
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //当
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //前
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[14][0],size,display);  //剩
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[15][0],size,display);  //余
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[30][0],size,display);  //金
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[31][0],size,display);  //额
    segpoint += (size/100);
    #endif    
}

/** 
 * @brief  假如是三相填充“当前透支电费”到中文提示区，假如是单相填充“当前透支金额”到中文提示区
 * @note   
 * @retval None
 */
static void Fill_OverdraftAmount_In_ChineseHintArea(void)
{
    unsigned int size;

    unsigned char segpoint;

    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //清空LCDRAM_Buf中的中文提示区
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //当
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //前
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[20][0],size,display);  //透
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[21][0],size,display);  //支
    segpoint += (size/100);   
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[23][0],size,display);  //电
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[34][0],size,display);  //费
    segpoint += (size/100);  
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //当
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //前
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[20][0],size,display);  //透
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[21][0],size,display);  //支
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[30][0],size,display);  //金
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[31][0],size,display);  //额
    segpoint += (size/100);
    #endif    
}


/** 
 * @brief  填充数值类数据到数字区
 * @note   
 * @param  valuepoint: 指向数值存储的数组，默认4字节BCD码，最低字节代表最低位
 * @param  decimalpoint: 代表显示的数值的小数位，0~7  
 * @param  plusminus: 代表是否显示负号，    Plus代表不显示，Minus代表显示
 * @param  displayhighzero: 高位是否显零，具体参见HIGHZERO_TYPE枚举
 * @retval None
 */
static void Fill_Value_In_NumberArea(unsigned char* valuepoint,unsigned char decimalpoint,PLUS_MINUS plusminus,HIGHZERO_TYPE displayhighzero)
{
    unsigned char i;
    unsigned char valuebuf[6];
    unsigned char displaynumber;
    unsigned char invalidzeronumber;
    unsigned char segpoint;

    Clear_NumberArea_Of_LCDRAM_Buf();    //清除LCDRAM_Buf的数字区

    for(i=0;i<4;i++)                     //复制数值数组
    {
        valuebuf[i] = *valuepoint;
        valuepoint++;        
    }
    
    if(displayhighzero == DisplayHighZero)                 //要求高位显0
    {
        displaynumber = 8;             //8个数字位都显示
    }
    else                               //要求高位不显0
    {
        invalidzeronumber=Get_InvalidZero_Number(valuebuf,4);     //得到无效零个数

        //根据小数位，判断个位数所在位置，因为加入即使高位不显零，但是个位数肯定要显示的
        if((8-invalidzeronumber)<=(decimalpoint+1))   
        {
            displaynumber = (decimalpoint+1);
        }
        else
        {
            displaynumber = (8-invalidzeronumber);
        }
    }
    segpoint = NumberAreaEndSeg+1;        //因为数字是靠右对齐
    
    for(i=0;i<displaynumber;i++)
    {
        if((decimalpoint == i)&&(decimalpoint != 0))     //如果没有小数，就不显示小数点，否则就要在相应位置插入小数点
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[0][0],Size_4P36P,display);     //显示小数点   
        }

        segpoint -= Size_18P36P/100;
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&NumberArea_char_18p36p[((valuebuf[i/2]>>((i%2)*4))&0x0f)][0],Size_18P36P,display); //显示数字
    }

    if(plusminus == Minus)       //显示负号
    {
        segpoint -= Size_8P36P/100;   
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&MinusIcon_8p36p[0][0],Size_8P36P,display);         //显示负号
    }       
}

/** 
 * @brief  根据日期数组填充数字区内容
 * @note   
 * @param  datepoint: 指向日期存储的数组，默认3字节BCD码，最低字节代表日
 * @retval None
 */
static void Fill_Date_In_NumberArea(unsigned char* datepoint)
{
    unsigned char i;
    unsigned char datebuf[3];
    unsigned char segpoint;

    Clear_NumberArea_Of_LCDRAM_Buf();           //清除LCDRAM_Buf的数字区
    
    //复制日期到datebuf数组
    for(i=0;i<3;i++)
    {
        datebuf[i] = *(datepoint+i);
    }
    
    segpoint = NumberAreaEndSeg+1;              //因为数字是靠右对齐

    for(i=0;i<6;i++)                            //显示6个数字
    {
        if((i == 2)||(i == 4))                  //在相应位置插入小数点,作为隔断
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[0][0],Size_4P36P,display);     //显示小数点   
        }

        segpoint -= Size_18P36P/100;
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&NumberArea_char_18p36p[((datebuf[i/2]>>((i%2)*4))&0x0f)][0],Size_18P36P,display); //显示数字
    }   
}
/** 
 * @brief  根据时间数组填充数字区内容
 * @note   
 * @param  timepoint: 
 * @retval None
 */
static void Fill_Time_In_NumberArea(unsigned char* timepoint)
{
    unsigned char i;
    unsigned char timebuf[3];
    unsigned char segpoint;

    Clear_NumberArea_Of_LCDRAM_Buf();           //清除LCDRAM_Buf的数字区
    
    //复制时间到datebuf数组
    for(i=0;i<3;i++)
    {
        timebuf[i] = *(timepoint+i);
    }
    
    segpoint = NumberAreaEndSeg+1;              //因为数字是靠右对齐

    for(i=0;i<6;i++)                            //显示6个数字
    {
        if((i == 2)||(i == 4))                  //在相应位置插入冒号,作为隔断
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[1][0],Size_4P36P,display);     //显示冒号   
        }

        segpoint -= Size_18P36P/100;
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&NumberArea_char_18p36p[((timebuf[i/2]>>((i%2)*4))&0x0f)][0],Size_18P36P,display); //显示数字
    }   
}

/** 
 * @brief  填充金额值到数字区
 * @note   
 * @param  amountpoint:     指向金额存储的数组，默认4字节BCD码，最低字节代表第1,2位小数
 * @param  plusminus:       代表是否显示负号，    plus代表不显示，minus代表显示
 * @param  displayhighzero: 高位是否显零，0代表不显示， 1代表显示
 * @retval None
 */
static void Fill_Amount_In_NumberArea(unsigned char* amountpoint,PLUS_MINUS plusminus,unsigned char displayhighzero)
{
    unsigned char i;
    unsigned char displaynumber;
    unsigned char invalidzeronumber;
    unsigned char amountbuf[3];
    unsigned char segpoint;

    Clear_NumberArea_Of_LCDRAM_Buf();           //清除LCDRAM_Buf的数字区
    
    //复制时间到datebuf数组
    for(i=0;i<4;i++)
    {
        amountbuf[i] = *(amountpoint+i);
    }
    
    segpoint = NumberAreaEndSeg+1;              //因为数字是靠右对齐

    if(displayhighzero == DisplayHighZero)      //要求高位显0
    {
        displaynumber = 8;                      //8个数字位都显示
    }
    else                                        //要求高位不显0
    {
        invalidzeronumber=Get_InvalidZero_Number(amountbuf,4);     //得到无效零个数

        //固定2位小数，所以最少显示3位小数
        if(invalidzeronumber>=5)   
        {
            displaynumber = 3;
        }
        else
        {
            displaynumber = (8-invalidzeronumber);//屏蔽无效零
        }
    }

    for(i=0;i<displaynumber;i++)                //显示6个数字
    {
        if(i == 2)                              //在相应位置插入小数点
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[0][0],Size_4P36P,display);     //显示小数点
        }
        segpoint -= Size_18P36P/100;
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&NumberArea_char_18p36p[((amountbuf[i/2]>>((i%2)*4))&0x0f)][0],Size_18P36P,display); //显示数字
    } 
    if(plusminus == Minus)                      //显示负号
    {
        segpoint -= Size_8P36P/100;   
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&MinusIcon_8p36p[0][0],Size_8P36P,display);     //显示负号   
    }

}




/** 
 * @brief  填充kwh到单位区
 * @note   
 * @retval None
 */
static void  Fill_Kwh_In_UnitArea(void)
{
    unsigned char segpoint;
    unsigned int size;

    Clear_UnitArea_Of_LCDRAM_Buf();             //清除LCDRAM_Buf的单位区

    size = Size_8P12P;
    //因为是靠右，所以要倒着写
    segpoint = UnitAreaEndSeg+1;
    segpoint -= (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_8p12p[4][0],size,display);    //h   
    segpoint -= (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_8p12p[0][0],size,display);    //W
    segpoint -= (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_8p12p[3][0],size,display);    //k

}

/** 
 * @brief  填充kvarh到单位区
 * @note   
 * @retval None
 */
static void Fill_kvah_In_UnitArea(void)
{
    unsigned char segpoint;
    unsigned int size;

    Clear_UnitArea_Of_LCDRAM_Buf();             //清除LCDRAM_Buf的单位区

    size = Size_25P12P;
    segpoint = UnitAreaEndSeg+1;
    segpoint -= (size/100);   
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_25p12p[0][0],size,display);    //kvarh
}

/** 
 * @brief  填充“元”到单位区
 * @note   
 * @retval None
 */
static void Fill_Yuan_In_UnitArea(void)
{
    unsigned char segpoint;
    unsigned int size;

    Clear_UnitArea_Of_LCDRAM_Buf();             //清除LCDRAM_Buf的单位区

    size = Size_12P12P;
    segpoint = UnitAreaEndSeg+1;
    segpoint -= (size/100);   
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_12p12p[1][0],size,display);    //元   
}

/*定义全局函数----------------------------------------------------------------*/
///定义可用于外部文件的函数
/*电表相关显示函数------------------------*/

/** 
 * @brief  显示电量函数
 * @note  
 * @param  phase: 代表相位 具体参见PHASE_TYPE枚举 
 * @param  engerytype: 电量种类，具体参见ENERGY_TYPE枚举
 * @param  date:    日期，暂时支持0~12，0表示当前  其他代表上x月
 * @param  rate:    费率 0~12，其中0代表总，其他代表Tx
 * @param  engerypoint: 指向电量存储的数组，默认6字节BCD码，最低字节代表第3,4小数
 * @param  decimalpoint: 代表显示的电量显示几位小数，0~4
 * @param  plusminus: 代表是否显示负号，    plus代表不显示，minus代表显示
 * @param  displayhighzero: 高位是否显零，具体参见HIGHZERO_TYPE枚举
 * @retval None
 */
extern void Display_Engery(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate,unsigned char* engerypoint,unsigned char decimalpoint,PLUS_MINUS plusminus,HIGHZERO_TYPE displayhighzero)
{
    unsigned char valuebuf[4];

    if((date>12)||(decimalpoint>4)||rate>19)    //月份超过12月，小数点超过4位或者费率超过19不支持，返回
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


/** 
 * @brief  显示当前日期
 * @note   
 * @param  datepoint: 指向日期存储的数组，默认3字节BCD码，最低字节代表日
 * @retval None
 */
extern void Display_CurrentDate(unsigned char* datepoint)
{

    //填充“当前日期”到中文提示内容
    Fill_CurrentDate_In_ChineseHintArea();
    //根据日期数组填充数字区内容
    Fill_Date_In_NumberArea(datepoint);

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
    Refresh_NumberArea_of_LCD_DDRAM();              //刷新到LCD的数字提示区
    Refresh_UnitArea_of_LCD_DDRAM();                //刷新到LCD的单位区(不能漏，确保单位区清空)
}

/** 
 * @brief  显示当前时间
 * @note   
 * @param  timepoint: 指向时间存储的数组，默认3字节BCD码，最低字节代表秒
 * @retval None
 */
extern void Display_CurrentTime(unsigned char* timepoint)
{
    //填充“当前时间”到中文提示内容
    Fill_CurrentTime_In_ChineseHintArea();
    //根据时间数组填充数字区内容
    Fill_Time_In_NumberArea(timepoint);

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
    Refresh_NumberArea_of_LCD_DDRAM();              //刷新到LCD的数字提示区
    Refresh_UnitArea_of_LCD_DDRAM();                //刷新到LCD的单位区(不能漏，确保单位区清空)
}


/** 
 * @brief  显示当前剩余金额
 * @note   三相显示“当前剩余电费” 单相显示“当前剩余金额”
 * @param  amountpoint: 指向剩余金额存储的数组，默认4字节BCD码，最低字节代表小数1、2位
 * @param  displayhighzero: 高位是否显零，0代表不显示， 1代表显示
 * @retval None
 */
extern void Display_RemainingAmount(unsigned char* amountpoint,unsigned char displayhighzero)
{
    //填充“当前剩余金额”或者“当前剩余电费”到中文提示内容
    Fill_RemainingAmount_In_ChineseHintArea();
    //根据剩余金额数组和其它参数填充数字区内容
    Fill_Amount_In_NumberArea(amountpoint,Plus,displayhighzero);
    //填充“元”到单位区
    Fill_Yuan_In_UnitArea();

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
    Refresh_NumberArea_of_LCD_DDRAM();              //刷新到LCD的数字提示区
    Refresh_UnitArea_of_LCD_DDRAM();                //刷新到LCD的单位区(不能漏，确保单位区清空)
}

/** 
 * @brief  显示当前透支金额
 * @note   三相显示“当前透支电费” 单相显示“当前透支金额”
 * @param  amountpoint: 指向透支金额存储的数组，默认4字节BCD码，最低字节代表小数1、2位
 * @param  displayhighzero: 高位是否显零，0代表不显示， 1代表显示
 * @retval None
 */
extern void Display_OverdraftAmount(unsigned char* amountpoint,unsigned char displayhighzero)
{
    //填充“当前透支金额”或者“当前透支电费”到中文提示内容
    Fill_OverdraftAmount_In_ChineseHintArea();
    //根据透支金额数组和其它参数填充数字区内容
    Fill_Amount_In_NumberArea(amountpoint,Minus,displayhighzero);
    //填充“元”到单位区
    Fill_Yuan_In_UnitArea();

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
    Refresh_NumberArea_of_LCD_DDRAM();              //刷新到LCD的数字提示区
    Refresh_UnitArea_of_LCD_DDRAM();                //刷新到LCD的单位区(不能漏，确保单位区清空)    
}

/*end-------------------------------------------------------------------------*/
