/*******************************************************************************
* @file      : Display.c
* @author    : Jiangsh
* @version   : V1.0.0
* @date      : Sat Apr 21 2018
* @brief     : 
********************************************************************************
* @attention :如果�?���?��存在汉字变量，文�??�?��成GB2312编码格式保存和打开
*
*
*/
/*头文�?----------------------------------------------------------------------*/
///添加头文�?
#include "Display.h"
#include "LCD.h"
#include "CharLib.h"
#include "LCDConfig.h"
#include "string.h"
/*宏定�?----------------------------------------------------------------------*/
///添加宏定�?
#define ChineseHintAreaGBKBufSize 20                //�?��提示区字�?BK码缓存区的大�?


typedef struct
{
    short GBKBuf[ChineseHintAreaGBKBufSize];        //字�?串GBK码缓存区
    int len;                                        //字�?串长�?
}GBKBUF_LENGTH_TYPE;                                //GBK编码缓存和长度结构体

/*内部变量声明----------------------------------------------------------------*/

/* �?��提示区LCD的�?份缓存数组，用于左右滚动显示 大小�?��文提示区缓存数组�?2�? */
static unsigned char ChineseHintArea_LCDRAM_BackupBuf[(ChineseHintAreaEndSeg-ChineseHintAreaStartSeg)*2][ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom];
/* �?��提示区的显示内�?的GBK缓存区和字�?串长�? */
static GBKBUF_LENGTH_TYPE ChineseHintAreaGBKStruct;
/* �?��提示区是否启用动态显示标志，0不启�?��1�?��表启�?��注意，初始化时默认启�?��每�?�?��文提示区时先清零 */
static unsigned char  ChineseHintArea_RollDisplay;


/*内部变量定义----------------------------------------------------------------*/



/*声明内部函数----------------------------------------------------------------*/
static unsigned char Get_InvalidZero_Number(unsigned char* buf,unsigned char len);
static void  Adjust_DecimalpointOfValue(unsigned char* srcbuf,unsigned char* objbuf,unsigned char decimalpoint);
const unsigned char* Get_CharBufAddress(unsigned short gbkcode);
static unsigned char  Get_GBKCodeOfStr(unsigned char* strpoint,unsigned short* gbkbufpoint);

static void  ChineseHintAreaGBKStruct_Init(void);


static void Clear_ChineseHintArea_LCDRAM_BackupBuf(void);
static  void ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit);
static void InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear);
static void Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf(unsigned char len,unsigned short offset);
static void Fill_Char_In_ChineseHintArea(unsigned char* strbuf,SPLITSCREENDISPLAY_TYPE SplitScreen);

/*定义内部函数----------------------------------------------------------------*/
/** 
 * @brief  获取数组�?��位无�?0的个�?
 * @note   数组必须为BCD格式，并默�?低字节数组存放低位数�?
 * @param  buf: �?���?��数组起�?地址
 * @param  len: �?���?��数据字节长度  0~10
 * @retval 无效0�?��
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

    number = 0;         //初�?�?

    for(i=0;i<len;i++)          //拷贝最高位字节数据并判�?��否为0，并�?��
    {
        temp = *(buf+len-1-i); 
        if((temp&0xf0) == 0x00)   //先判�?��位：等于0，就无效0数加1，否则就直接返回
        {
        number++;
        }
        else                    //碰到非零，结束判�?��并返�?
        {
            return number;
        }            
        if((temp&0x0f) == 0x00)   //再判�?��位：等于0，就无效0数加1，否则就直接返回
        {
            number++;
        }
        else                    //碰到非零，结束判�?��并返�?
        {
            return number;
        }

    }
    
    return number;              //�?��完了，也返回
}
/** 
 * @brief  调整4位小数的6字节数组保留的小数位复制到目标数�?
 * @note   注意源数组的格式，必须满�?6字节，并默�?最低字�?2�?��组元素为小数
 *         该函数只要是为了处理4位小数的电量
 * @param  srcbuf: 6字节数组，最低字�?2�?��组元素为小数，BCD格式
 * @param  objbuf: 4字节数组，BCD格式
 * @param  decimalpoint: 准�?保留的小数个数，0~4
 * @retval None
 */
static void  Adjust_DecimalpointOfValue(unsigned char* srcbuf,unsigned char* objbuf,unsigned char decimalpoint)
{
    
    if(decimalpoint>4)  //超范围了，直接返�?
    {
        return;
    }

    switch(decimalpoint)//根据小数位调整目标数�?
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
 * @brief  根据GBK码返回字符字库内容所在位�?
 * @note   
 * @param  gbkcode: gbk�?
 * @retval 所在位�?��指针
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
    //找不到，就返回数�?0的位�?
    #if (MeterType == ThreePhaseMeter)
    return &ChineseHint_Char_6p12p[0][0];
    #else
    return &ChineseHint_Char_7p14p[0][0];    
    #endif
}
/** 
 * @brief   将字符串�?��成GBK�?
 * @note   
 * @param  strpoint: 字�?串指�?
 * @param  gbkbufpoint: �?��成的GBK码存放数组的地址指针
 * @retval GBK码数�?
 */
static unsigned char  Get_GBKCodeOfStr(unsigned char* strpoint)
{
    unsigned char i;
    unsigned char charnumberpoint=0;
    
    for(i=0;i<42;i++)           //赋值字符串的GBK代码解析并转存，同时反�?长度
    {
        if(*(strpoint+i) == 0)  //碰到\n 代表已经结束
        {
           return charnumberpoint;              //退�?
        }
        else if(((*(strpoint+i))&0x80) == 0x80)      //最高位�?1，代表是汉字编码一部分
        {
            i++;                            
            if(((*(strpoint+i))&0x80) == 0x80)      //最高位�?1，代表是汉字编码一部分
            {
               *gbkbufpoint =  (*(strpoint+i-1))*256+(*(strpoint+i));       //得到2字节的GBK�?
            }
            else                           //前一�?��汉字编码一部分，后一�?���?��不�?合�?律，不解�?
            {
                continue;                  //停�?这一次，继续下一次循�?
            }            
        }
        else                              //最高位不是1，且不是\n 就代表是字母或者数�?
        {
            *gbkbufpoint =  *(strpoint+i); //得到2字节的GBK码，高位字节�?00
        }        
        //能执行到这里，�?明一�?��符解析成�?
        gbkbufpoint++;                      //GBK编码数组指针�?1
        charnumberpoint++;                  //有效字�?数加1 
    }

    return charnumberpoint;              //退�?


}

/** 
 * @brief  �?��提示区的GBK码缓存区初�?�?
 * @note   
 * @retval None
 */
static void  ChineseHintAreaGBKStruct_Init(void)
{
    int i;
    for(i=0;i<ChineseHintAreaGBKBufSize;i++)
    {
        ChineseHintAreaGBKStruct.GBKBuf[i] = 0;
    }

    ChineseHintAreaGBKStruct.len = 0;
}

/** 
 * @brief  根据字�?串内容，�?��成GBK码存放到ChineseHintAreaGBKStruct�?
 * @note   
 * @param  number: ChineseHintAreaGBKStruct缓存数组的下�?
 * @param  *str: 字�?�?
 * @retval None
 */
static void Write_Str_ChineseHintAreaGBKStruct(int number,char *str)
{
    unsigned char i;

    int offset=0;
    
    for(i=0;i<(42-number);i++)           //赋值字符串的GBK代码解析并转存，同时反�?长度
    {
        if(*(str+i) == 0)  //碰到\n 代表已经结束
        {
           break;              //退�?
        }
        else if(((*(str+i))&0x80) == 0x80)      //最高位�?1，代表是汉字编码一部分
        {
            i++;                                //汉字�?2字节                  
            ChineseHintAreaGBKStruct.GBKBuf[number+offset] =  (*(str+i-1))*256+(*(str+i));       //得到2字节的GBK�?
                     
        }
        else                                //最高位不是1，且不是\n 就代表是字母或者数字，�?��1字节
        {
            ChineseHintAreaGBKStruct.GBKBuf[number+offset] =  *(str+i);  //得到2字节的GBK码，高位字节�?00
        }        
        //能执行到这里，�?明一�?��符解析成�?
        offset++;                           //GBK编码数组指针�?1
    }

    //str的字符串全部读出来了，或者数组全满了
    if(i == (42-number))
    {
      ChineseHintAreaGBKStruct.len =  ChineseHintAreaGBKBufSize; 
    }
    else
    {
        ChineseHintAreaGBKStruct.len =  strlen(str);
    }

}


/** 
 * @brief  将一个gbk编码存放到ChineseHintAreaGBKStruct�?
 * @note   
 * @param  number: ChineseHintAreaGBKStruct缓存数组的下�?
 * @param  gbkcode: gbk编码
 * @retval None
 */
static void Write_Gbk_ChineseHintAreaGBKStruct(int number,short gbkcode)
{
    ChineseHintAreaGBKStruct.GBKBuf[number+offset] =  (*(str+i-1))*256+(*(str+i));       //得到2字节的GBK�?    
}

/** 
 * @brief  获取ChineseHintAreaGBKStruct有效GBK码的�?��
 * @note   
 * @retval GBK码个�?
 */
static int Get_ChineseHintAreaGBKStruct_Len(void)
{
    int i;

    ChineseHintAreaGBKStruct.len=0;

    for(i=0;i<ChineseHintAreaGBKBufSize;i++)
    {

        if(ChineseHintAreaGBKStruct.GBKBuf[i] != 0 )
        {
            ChineseHintAreaGBKStruct++;
        }
        else
        {
            break;
        }
    }
}

/*操作LCDRAM_BackupBuf数组函数------------------------*/
/** 
 * @brief  清空�?��提示区�?份缓存数组内�?
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
 * @note   (x,y)位置的某点，显示或者不显示�?
 * 注意，这�?���?��了�?用缓存区的点，最终显示需要调用其它函数将备用缓存区写到缓存区，再将缓存区数据写到LCD
 * @param  x: seg的位�?
 * @param  y: com的位�?
 * @param  bit: 0：取�? 1：�?�?
 * @retval None
 */
static  void ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit)
{
    unsigned short pos;
    unsigned short bx;
    unsigned char temp=0;

    if((x>=(seg*2))||y>=com)return;       //超出范围
    pos=y/8;        //得到页地址       
    
    bx=y%8;         //得到点在页地址的bit�?
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
 * @brief  往ChineseHintArea_LCDRAM_BackupBuf的指定位�?��示或清除一�?���?(包括汉字、字母、数字、�?�?)
 * @note   描点方式�?��列式，清除的话，确保同一位置显示了�?应字符，不然不保证清除效�?
 * @param  x: 指定起�?位置的seg
 * @param  y: 指定起�?位置的com
 * @param  charbufstartaddress: 字�?的字模起始地址
 * @param  size: 字�?尺�?
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
    bytesnumber = segnumber*(comnumber/8+((comnumber%8)?1:0));      //得到需要显示所用的字节�?
    
    for(t=0;t<bytesnumber;t++)                                      //将显示所用的字节全部写到LCDRAM_Buf数组�?
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
                if(displayorclear == display)      //�?1，即显示
                {
                    ChineseHintArea_LCDRAM_BackupBuf_DrawPoint(x,y,1);
                }
                else                                //�?0，即清除
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
 * @note   ChineseHintArea_LCDRAM_BackupBuf的起始位�?+offset开始拷贝到LCDRAM_Buf[ChineseHintAreaStartSeg~ChineseHintAreaStartSeg+len]
 * @param  len: 拷贝的长�?
 * @param  offset: 偏移�?
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
 * @brief  �?��内�?到中文提示区的缓存数组和备用数组
 * @note   这里先写到�?用缓存数组然后判�?��否超过缓存数组大小，假�?超过，就�?��动态显�?
 * @param  strbuf: 字�?串变量起始地址
 * @param  SplitScreen: 分屏�?��显示或者显示几号分�?
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
    
    //初�?�?
    for(i=0;i<ChineseHintAreaGBKBufSize;i++)
    {
        ChineseHintAreaGBKBuf[i] = 0;   //GBK码缓存区清零
    }

    ChineseHintArea_RollDisplay = 0;                //滚动显示标志位清�?
    ChineseHintAreaGBKLen = 0;                      //有效GBK码数量清�?
    Clear_ChineseHintArea_Of_LCDRAM_Buf();          //字模内�?缓存区清�?
    Clear_ChineseHintArea_LCDRAM_BackupBuf();      //字模内�?备份缓存区清�?

    //判断分屏�?��显示
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

        segendpoint = SplitWindowAreaStartseg;      //�?���?��提示区文字内容的结束seg地址
    }
    else
    {
        segendpoint = ChineseHintAreaEndSeg;        //�?���?��提示区文字内容的结束seg地址
    }

    //�?���?��提示区文字内�?
    #if (MeterType == ThreePhaseMeter)
    sizenumber = Size_6P12P;
    size = Size_12P12P;
    #else
    sizenumber = Size_7P14P;
    size = Size_14P14P;
    #endif

    ChineseHintAreaGBKLen = Get_GBKCodeOfStr(strbuf,ChineseHintAreaGBKBuf);    //得到有效GBK码和数量

    if(ChineseHintAreaGBKLen>ChineseHintAreaGBKBufSize) //先判�?��效GBK码数量，默�?�?��ChineseHintAreaGBKBufSize个GBK码，如果超过，有�?��Clear_LCDRAM_BackupBuf不�?�?��就�?直接返回，不继续执�?
    {
        return;
    }
    else            //没有超过，就将内容写到�?用缓存数组中
    {
        segpoint = 0;       //备用缓存数组�?��起�?地址开始写
        
        for(i=0;i<ChineseHintAreaGBKLen;i++)
        {
            addresspoint = Get_CharBufAddress(ChineseHintAreaGBKBuf[i]);    //得到字�?所在地址

            if(segpoint>(2*seg))        //超过备用缓存区数组，就返�?
            {
                break;
            }

            if((ChineseHintAreaGBKBuf[i]&0x8080) == 0x8080 )    //汉字
            {
               InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(segpoint,ChineseHintAreaStartCom,addresspoint,size,display);  
               segpoint += size/100;
            }
            else                                                //字母和数�?
            {
               InputCharacter_to_ChineseHintArea_LCDRAM_BackupBuf(segpoint,ChineseHintAreaStartCom,addresspoint,sizenumber,display);   
               segpoint += sizenumber/100;    
            }
        }  

        //将GBK全部�?��成字模内容后，就要判�?��部内容有没有超中文提示区范围，假如超了就要启用滚动显�?
        Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf((segendpoint-ChineseHintAreaStartSeg),0);
        if(segpoint>=segendpoint)  //超了
        {
            ChineseHintArea_RollDisplay =1;     //�?��滚动显示
        }
    }
}
/** 
 * @brief  �?��“当前组合有功总电量”或者“当前有功总电量”到�?��提示�?
 * @note   不显示分�?
 * @retval None
 */
static void Fill_CurrentCombinedActivePowerEnergyT0(void)
{
    unsigned char* str;
    #if (MeterType == ThreePhaseMeter)
    str = "当前组合有功总电�?";
    #else
    str = "当前有功总电�?";
    #endif
    Fill_Char_In_ChineseHintArea(str,NoDisplaySplitScreen);
}

/** 
 * @brief  �?��“当前”或者“上x月”到�?��提示�?
 * @note   
 * @param  startseg: 显示的起始位�?
 * @param  date: 日期，暂时支�?0~12�?0表示当前  其他代表上x�?
 * @retval 返回的是写完后seg所在位�?
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
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //�?
        segpoint += (size/100);
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //�?
        segpoint += (size/100);
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //�?
        segpoint += (size/100);
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //�?
        segpoint += (size/100);
        #endif
    }
    else                   //显示上X�?
    {
        //获取月份的高低字节BCD�?
        high=date/10;
        low=date%10;
       
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[2][0],size,display);   //�?
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[2][0],size,display);   //�?
        #endif
        segpoint += (size/100);

        //显示XX
        if(high != 0)       //高位不为0，就需要显�?
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
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[3][0],size,display);   //�?
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[3][0],size,display);   //�?
        #endif
        segpoint += (size/100);
    }

    return segpoint;
}

/** 
 * @brief  �?��“A相”“B相”“C相”到�?��提示�?
 * @note   合相不显�?
 * @param  startseg: 显示的起始位�?
 * @param  phase: 相位类型，具体参见PHASE_TYPE枚举
 * @retval 返回的是写完后seg所在位�?
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
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[52][0],size,display);   //�?
        segpoint += (size/100);
    }
    #endif

    return segpoint;
}
/** 
 * @brief  �?��“组合有功”“�?向有功”“反向有功”“无功组�?1”“无功组�?2”“�?向无功”“反向无功”“无功Ⅰ”“无功Ⅱ”“无功Ⅲ”“无功Ⅳ”“�?向�?在”“反向�?在”到�?��提示�?
 * @note   
 * @param  startseg: 字�?显示起�?位置
 * @param  engerytype: 电量种类，具体参见ENERGY_TYPE枚举
 * @retval 返回的是写完后seg所在位�?
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

    if((engerytype == CombinedActivePowerEnergy)||(engerytype == PositiveActivePowerEnergy)||(engerytype == ReverseActivePowerEnergy))     //有功�?
    {

        if(engerytype == PositiveActivePowerEnergy)      //正向有功�?
        {           
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[6][0],size,display);   //�?
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[6][0],size,display);   //�?
            #endif
            segpoint += (size/100);             
        }
        else if(engerytype == ReverseActivePowerEnergy) //反向有功�?   
        {  
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[7][0],size,display);   //�?
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[7][0],size,display);   //�?
            #endif
            segpoint += (size/100);             
        }
        
        if(engerytype != CombinedActivePowerEnergy) //不是组合，就显示“向�?
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display);   //�?
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display);     //�?
            #endif
            segpoint += (size/100);  
        }

        #if (MeterType == ThreePhaseMeter)          //单相表不需要显示有�?
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[37][0],size,display); //�?
        segpoint += (size/100); 
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[38][0],size,display);//�?
        segpoint += (size/100);   
        #endif   
    }
    else if((engerytype == CombinedOneReactivePowerEnergy)||(engerytype == CombinedTwoReactivePowerEnergy)\
          ||(engerytype == PositiveReactivePowerEnergy)||(engerytype == ReverseReactivePowerEnergy)\
          ||(engerytype == FirstQuadrantReactivePowerEnergy)||(engerytype == SecondQuadrantReactivePowerEnergy)\
          ||(engerytype == ThirdQuadrantReactivePowerEnergy)||(engerytype == FourthQuadrantReactivePowerEnergy))     //无功�?
    {
        combinedreactivepowerenergy = 0;

        if((engerytype == CombinedOneReactivePowerEnergy)||(engerytype == CombinedTwoReactivePowerEnergy))      //组合无功�?
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[4][0],size,display); //�?
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[5][0],size,display); //�?
            segpoint += (size/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[4][0],size,display); //�?
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[5][0],size,display); //�?
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
        else if(engerytype == PositiveReactivePowerEnergy) //正向无功�?
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[6][0],size,display); //�?
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display); //�?
            segpoint += (size/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[6][0],size,display); //�?
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display); //�?
            segpoint += (size/100); 
            #endif

        }
        else if(engerytype == ReverseReactivePowerEnergy) //反向无功�?
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[7][0],size,display); //�?
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display); //�?
            segpoint += (size/100); 
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[7][0],size,display); //�?
            segpoint += (size/100); 
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display); //�?
            segpoint += (size/100); 
            #endif            

        }  
        else if((engerytype == FirstQuadrantReactivePowerEnergy)||(engerytype == SecondQuadrantReactivePowerEnergy)\
              ||(engerytype == ThirdQuadrantReactivePowerEnergy)||(engerytype == FourthQuadrantReactivePowerEnergy)) //象限无功�?  
        {
            if(engerytype == FirstQuadrantReactivePowerEnergy)  //�?��象限
            {

                /*
                //显示“I�?
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[44][0],size,display); //�?(44)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[44][0],size,display); //�?(44)
                segpoint += (size/100);     
                #endif   
                */
                //显示“�?1象限�?
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[53][0],size,display);      //�?(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[1][0],sizenumber,display); //1
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //�?(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //�?(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[53][0],size,display);      //�?(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[1][0],sizenumber,display); //1
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //�?(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //�?(55) 
                segpoint += (size/100);      
                #endif                  

            }
            else if(engerytype == SecondQuadrantReactivePowerEnergy)  //�?��象限
            {
                /*
                //显示“Ⅱ�?
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[45][0],size,display); //�?(45)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[45][0],size,display); //�?(45)
                segpoint += (size/100);     
                #endif   
                */
                //显示“�?2象限�?
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //�?(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[2][0],sizenumber,display);     //2
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //�?(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[56][0],size,display);      //�?(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //�?(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[2][0],sizenumber,display); //2
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //�?(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[56][0],size,display);      //�?(55) 
                segpoint += (size/100);      
                #endif 
            }
            else if(engerytype == ThirdQuadrantReactivePowerEnergy)  //�?��象限
            {
                /*
                //显示“Ⅲ�?
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Chinese_12p12p[46][0],size,display); //�?(46)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[46][0],size,display); //�?(46)
                segpoint += (size/100);     
                #endif 
                */
                //显示“�?3象限�?
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //�?(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[3][0],sizenumber,display); //3
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //�?(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[56][0],size,display);      //�?(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //�?(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[3][0],sizenumber,display); //3
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //�?(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[56][0],size,display);      //�?(55) 
                segpoint += (size/100);      
                #endif                

            }
            else if(engerytype == FourthQuadrantReactivePowerEnergy)  //�?��象限
            {
                /*
                //显示“Ⅳ�?
                #ifdef ThreePhase
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Chinese_12p12p[47][0],size,display); //�?(47)
                segpoint += (size/100); 
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[47][0],size,display); //�?(47)
                segpoint += (size/100);     
                #endif  
                */
               
                //显示“�?4象限�?
                #if (MeterType == ThreePhaseMeter)
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[54][0],size,display);      //�?(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_6p12p[4][0],sizenumber,display); //4
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[55][0],size,display);      //�?(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[56][0],size,display);      //�?(55) 
                segpoint += (size/100);                   
                #else
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[54][0],size,display);      //�?(53) 
                segpoint += (size/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_7p14p[4][0],sizenumber,display); //4
                segpoint += (sizenumber/100); 
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[55][0],size,display);      //�?(54) 
                segpoint += (size/100);                
                InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[56][0],size,display);      //�?(55) 
                segpoint += (size/100);      
                #endif                 
            }
        } 
        
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[36][0],size,display); //�?
        segpoint += (size/100); 
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[38][0],size,display);  //�?
        segpoint += (size/100); 
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[36][0],size,display); //�?
        segpoint += (size/100); 
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[38][0],size,display);  //�?
        segpoint += (size/100);   
        #endif  


        //显示组合无功1或者无�?2�?��数字
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
    else if((engerytype == PositiveApparentEnergy)||(engerytype == ReverseApparentEnergy))      //视在�?
    {
        if(engerytype == PositiveApparentEnergy)        //正向视在
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[6][0],size,display);   //�?
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[6][0],size,display);   //�?
            #endif
            segpoint += (size/100); 
        }
        else                                            //反向视在
        {
            #if (MeterType == ThreePhaseMeter)
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[7][0],size,display);   //�?
            #else
            InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[7][0],size,display);   //�?
            #endif
            segpoint += (size/100); 
        }

        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[8][0],size,display);   //�?
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[8][0],size,display);   //�?
        #endif
        segpoint += (size/100); 
        
        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[50][0],size,display);   //�?
        segpoint += (size/100);  
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[51][0],size,display);   //�?
        segpoint += (size/100);  
        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[50][0],size,display);         //�?
        segpoint += (size/100);  
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[51][0],size,display);        //�?
        segpoint += (size/100);  
        #endif 
    }   
    return segpoint;
}

/** 
 * @brief  �?��"总电�?"或者“Tx电量”到�?��提示�?
 * @note   
 * @param  startseg: 字�?显示起�?位置
 * @param  rate: 费率 0~12，其�?0代表总，其他代表Tx
 * @retval 返回的是写完后seg所在位�?
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
        //获取费率的高低字节BCD�?
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
    else             //总电�?
    {

        #if (MeterType == ThreePhaseMeter)
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[9][0],size,display); //�?
        segpoint += (size/100); 

        #else
        InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[9][0],size,display); //�?
        segpoint += (size/100); 
        #endif 

    }    

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[23][0],size,display); //�?
    segpoint += (size/100); 
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[24][0],size,display); //�?
    segpoint += (size/100); 
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[23][0],size,display); //�?
    segpoint += (size/100); 
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[24][0],size,display); //�?
    segpoint += (size/100); 
    #endif 

    return segpoint;
}
/** 
 * @brief  根据电量种类和费率判�??�?��的中文提示内�?
 * @note   
 * @param  phase: 相位，具体参见PHASE_TYPE枚举
 * @param  engerytype: 电量种类，具体参见ENERGY_TYPE枚举
 * @param  date: 日期，暂时支�?0~12�?0表示当前  其他代表上x�?
 * @param  rate: 费率 0~12，其�?0代表总，其他代表Tx
 * @retval None
 */
static void Fill_Engery_In_ChineseHintArea(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate)
{
    unsigned char i;
    unsigned char segpoint;
    char* str;
    int len;
    Clear_ChineseHintArea_Of_LCDRAM_Buf();      //清空LCDRAM_Buf�?���?��提示�?
    Clear_ChineseHintArea_LCDRAM_BackupBuf();   //清零�?��提示区的备用数组
    ChineseHintAreaGBKStruct_Init();            //�?��提示区的GBK码缓存数组和有效GBK码数量清�?
    len = 0;
    //时间
    switch(date)
    {
        case 0:
            str = "当前";
        break;
        case 1:
            str = "�?1�?";
        break;
        case 2:
            str = "�?2�?";
        break;
        case 3:
            str = "�?3�?";
        break;
        case 4:
            str = "�?4�?";
        break;
        case 5:
            str = "�?5�?";
        break;
        case 6:
            str = "�?6�?";
        break;
        case 7:
            str = "�?7�?";
        break;
        case 8:
            str = "�?8�?";
        break;
        case 9:
            str = "�?9�?";
        break;
        case 10:
            str = "�?10�?";
        break;
        case 11:
            str = "�?11�?";
        break;
        case 12:
            str = "�?12�?";
        break;
        case 13:
            str = "�?13�?";
        break;
        case 14:
            str = "�?14�?";
        break;
        case 15:
            str = "�?15�?";
        break;
        case 16:
            str = "�?16�?";
        break;
        case 17:
            str = "�?17�?";
        break;
        case 18:
            str = "�?18�?";
        break;
        case 19:
            str = "�?19�?";
        break;
        case 20:
            str = "�?20�?";
        break;
        case 21:
            str = "�?21�?";
        break;
        case 22:
            str = "�?22�?";
        break;
        case 23:
            str = "�?23�?";
        break;
        case 24:
            str = "�?24�?";
        break;
        default:
            str = "0";
        break;
    }
    
    Write_Str_ChineseHintAreaGBKStruct(0,str);          //�?��提示区的GBK码缓存数组起始地址存储str字�?串的gbk�?
    len += strlen(str);                                 //�?��字�?串长�?
    //相位
    switch(phase)
    {   
        case APhase:
            str = "A�?";
        break;
        
        case BPhase:
            str = "B�?";
        break;
        
        case CPhase:
            str = "C�?";
        break;

        default:
            str = "0";
        break;
    }
    if(phase != TotalPhase)
    {
        Write_Str_ChineseHintAreaGBKStruct(len,str);      //�?��提示区的GBK码缓存数组起始地址存储str字�?串的gbk�?
        len += strlen(str);                               //�?��字�?串长�?        
    }

    //电量种类
    switch(engerytype)
    {
        case CombinedActivePowerEnergy:
            #if (MeterType == ThreePhaseMeter)
            str = "组合有功";
            #else
            str = "有功";
            #endif
        break;
        case PositiveActivePowerEnergy:
            str = "正向有功";
        break;
        case ReverseActivePowerEnergy:
            str = "反向有功";
        break;
        case CombinedOneReactivePowerEnergy:
            str = "组合无功1";
        break;
        case CombinedTwoReactivePowerEnergy:
            str = "组合无功2";
        break;
        case PositiveReactivePowerEnergy:
            str = "正向无功";
        break;
        case ReverseReactivePowerEnergy:
            str = "反向无功";
        break;
        case FirstQuadrantReactivePowerEnergy:
            str = "�?1象限无功";
        break;
        case SecondQuadrantReactivePowerEnergy:
            str = "�?2象限无功";
        break;
        case ThirdQuadrantReactivePowerEnergy:
            str = "�?3象限无功";
        break;
        case FourthQuadrantReactivePowerEnergy:
            str = "�?4象限无功";
        break;
        case PositiveApparentEnergy:
            str = "正向视在";
        break;
        case ReverseApparentEnergy:
            str = "反向视在";
        break;
        default:
            str = "0"
        break;
    }
    Write_Str_ChineseHintAreaGBKStruct(len,str);        //�?��提示区的GBK码缓存数组起始地址存储str字�?串的gbk�?
    len += strlen(str);                                 //�?��字�?串长�?    
    
    //费率
    switch(date)
    {
        case 0:
            str = "�?";
        break;
        case 1:
            str = "T1";
        break;
        case 2:
            str = "T2";
        break;
        case 3:
            str = "T3";
        break;
        case 4:
            str = "T4";
        break;
        case 5:
            str = "T5";
        break;
        case 6:
            str = "T6";
        break;
        case 7:
            str = "T7";
        break;
        case 8:
            str = "T8";
        break;
        case 9:
            str = "T9";
        break;
        case 10:
            str = "T10";
        break;
        case 11:
            str = "T11";
        break;
        case 12:
            str = "T12";
        break;
        case 13:
            str = "T13";
        break;
        case 14:
            str = "T14";
        break;
        default:
            str = "0"
        break;
    }

    Write_Str_ChineseHintAreaGBKStruct(len,str);        //�?��提示区的GBK码缓存数组起始地址存储str字�?串的gbk�?
    len += strlen(str);  

    str = "电量"
    Write_Str_ChineseHintAreaGBKStruct(len,str);        //�?��提示区的GBK码缓存数组起始地址存储str字�?串的gbk�?
    len += strlen(str);  

}
/** 
 * @brief  �?��“当前日期”到�?��提示�?
 * @note   
 * @retval None
 */
static void Fill_CurrentDate_In_ChineseHintArea(void)
{
    unsigned int size;

    unsigned char segpoint;
    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //清空LCDRAM_Buf�?���?��提示�?
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[48][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[49][0],size,display);   //�?
    segpoint += (size/100);    
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[48][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[49][0],size,display);   //�?
    segpoint += (size/100);
    #endif
}

/** 
 * @brief  �?��“当前时间”到�?��提示�?
 * @note   
 * @retval None
 */
static void Fill_CurrentTime_In_ChineseHintArea(void)
{
    unsigned int size;
    unsigned char segpoint;
    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //清空LCDRAM_Buf�?���?��提示�?
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[27][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[28][0],size,display);   //�?
    segpoint += (size/100);    
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[27][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[28][0],size,display);   //�?
    segpoint += (size/100);
    #endif    
}

/** 
 * @brief  假�?�?��相填充“当前剩余电费”到�?��提示区，假�?�?��相填充“当前剩余金额”到�?��提示�?
 * @note   
 * @retval None
 */
static void Fill_RemainingAmount_In_ChineseHintArea(void)
{
    unsigned int size;
    unsigned char segpoint;

    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //清空LCDRAM_Buf�?���?��提示�?
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[14][0],size,display);  //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[15][0],size,display);  //�?
    segpoint += (size/100);   
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[23][0],size,display);  //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[34][0],size,display);  //�?
    segpoint += (size/100);  
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[14][0],size,display);  //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[15][0],size,display);  //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[30][0],size,display);  //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[31][0],size,display);  //�?
    segpoint += (size/100);
    #endif    
}

/** 
 * @brief  假�?�?��相填充“当前透支电费”到�?��提示区，假�?�?��相填充“当前透支金�?”到�?��提示�?
 * @note   
 * @retval None
 */
static void Fill_OverdraftAmount_In_ChineseHintArea(void)
{
    unsigned int size;

    unsigned char segpoint;

    
    Clear_ChineseHintArea_Of_LCDRAM_Buf();  //清空LCDRAM_Buf�?���?��提示�?
   
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    #else
    size = Size_14P14P;
    #endif
    
    segpoint = ChineseHintAreaStartSeg;     

    #if (MeterType == ThreePhaseMeter)
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[0][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[1][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[20][0],size,display);  //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[21][0],size,display);  //�?
    segpoint += (size/100);   
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[23][0],size,display);  //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_12p12p[34][0],size,display);  //�?
    segpoint += (size/100);  
    #else
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[0][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[1][0],size,display);   //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[20][0],size,display);  //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[21][0],size,display);  //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[30][0],size,display);  //�?
    segpoint += (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,ChineseHintAreaStartCom,&ChineseHint_Char_14p14p[31][0],size,display);  //�?
    segpoint += (size/100);
    #endif    
}


/** 
 * @brief  �?��数值类数据到数字区
 * @note   
 * @param  valuepoint: 指向数值存储的数组，默�?4字节BCD码，最低字节代表最低位
 * @param  decimalpoint: 代表显示的数值的小数位，0~7  
 * @param  plusminus: 代表�?��显示负号�?    Plus代表不显示，Minus代表显示
 * @param  displayhighzero: 高位�?��显零，具体参见HIGHZERO_TYPE枚举
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

    for(i=0;i<4;i++)                     //复制数值数�?
    {
        valuebuf[i] = *valuepoint;
        valuepoint++;        
    }
    
    if(displayhighzero == DisplayHighZero)                 //要求高位�?0
    {
        displaynumber = 8;             //8�?��字位都显�?
    }
    else                               //要求高位不显0
    {
        invalidzeronumber=Get_InvalidZero_Number(valuebuf,4);     //得到无效零个�?

        //根据小数位，判断�?��数所在位�?��因为加入即使高位不显零，但是�?��数肯定�?显示�?
        if((8-invalidzeronumber)<=(decimalpoint+1))   
        {
            displaynumber = (decimalpoint+1);
        }
        else
        {
            displaynumber = (8-invalidzeronumber);
        }
    }
    segpoint = NumberAreaEndSeg+1;        //因为数字�?��右�?�?
    
    for(i=0;i<displaynumber;i++)
    {
        if((decimalpoint == i)&&(decimalpoint != 0))     //如果没有小数，就不显示小数点，否则就要在相应位置插入小数�?
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[0][0],Size_4P36P,display);     //显示小数�?   
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
 * @brief  根据日期数组�?��数字区内�?
 * @note   
 * @param  datepoint: 指向日期存储的数组，默�?3字节BCD码，最低字节代表日
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
    
    segpoint = NumberAreaEndSeg+1;              //因为数字�?��右�?�?

    for(i=0;i<6;i++)                            //显示6�?���?
    {
        if((i == 2)||(i == 4))                  //在相应位�?��入小数点,作为隔断
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[0][0],Size_4P36P,display);     //显示小数�?   
        }

        segpoint -= Size_18P36P/100;
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&NumberArea_char_18p36p[((datebuf[i/2]>>((i%2)*4))&0x0f)][0],Size_18P36P,display); //显示数字
    }   
}
/** 
 * @brief  根据时间数组�?��数字区内�?
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
    
    segpoint = NumberAreaEndSeg+1;              //因为数字�?��右�?�?

    for(i=0;i<6;i++)                            //显示6�?���?
    {
        if((i == 2)||(i == 4))                  //在相应位�?��入冒�?,作为隔断
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[1][0],Size_4P36P,display);     //显示冒号   
        }

        segpoint -= Size_18P36P/100;
        InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&NumberArea_char_18p36p[((timebuf[i/2]>>((i%2)*4))&0x0f)][0],Size_18P36P,display); //显示数字
    }   
}

/** 
 * @brief  �?��金�?值到数字�?
 * @note   
 * @param  amountpoint:     指向金�?存储的数组，默�?4字节BCD码，最低字节代表�?1,2位小�?
 * @param  plusminus:       代表�?��显示负号�?    plus代表不显示，minus代表显示
 * @param  displayhighzero: 高位�?��显零�?0代表不显示， 1代表显示
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
    
    segpoint = NumberAreaEndSeg+1;              //因为数字�?��右�?�?

    if(displayhighzero == DisplayHighZero)      //要求高位�?0
    {
        displaynumber = 8;                      //8�?��字位都显�?
    }
    else                                        //要求高位不显0
    {
        invalidzeronumber=Get_InvalidZero_Number(amountbuf,4);     //得到无效零个�?

        //固定2位小数，所以最少显�?3位小�?
        if(invalidzeronumber>=5)   
        {
            displaynumber = 3;
        }
        else
        {
            displaynumber = (8-invalidzeronumber);//屏蔽无效�?
        }
    }

    for(i=0;i<displaynumber;i++)                //显示6�?���?
    {
        if(i == 2)                              //在相应位�?��入小数点
        {
            segpoint -= Size_4P36P/100;  
            InputCharacter_to_LCDRAM_Buf(segpoint,NumberAreaStartCom,&Point_4p36p[0][0],Size_4P36P,display);     //显示小数�?
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
 * @brief  �?��kwh到单位区
 * @note   
 * @retval None
 */
static void  Fill_Kwh_In_UnitArea(void)
{
    unsigned char segpoint;
    unsigned int size;

    Clear_UnitArea_Of_LCDRAM_Buf();             //清除LCDRAM_Buf的单位区

    size = Size_8P12P;
    //因为�?��右，所以�?倒着�?
    segpoint = UnitAreaEndSeg+1;
    segpoint -= (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_8p12p[4][0],size,display);    //h   
    segpoint -= (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_8p12p[0][0],size,display);    //W
    segpoint -= (size/100);
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_8p12p[3][0],size,display);    //k

}

/** 
 * @brief  �?��kvarh到单位区
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
 * @brief  �?��“元”到单位�?
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
    InputCharacter_to_LCDRAM_Buf(segpoint,UnitAreaStartCom,&UintArea_char_12p12p[1][0],size,display);    //�?   
}

/*定义全局函数----------------------------------------------------------------*/
///定义�?��于�?部文件的函数
/*电表相关显示函数------------------------*/

/** 
 * @brief  显示电量函数
 * @note  
 * @param  phase: 代表相位 具体参�?PHASE_TYPE枚举 
 * @param  engerytype: 电量种类，具体参见ENERGY_TYPE枚举
 * @param  date:    日期，暂时支�?0~12�?0表示当前  其他代表上x�?
 * @param  rate:    费率 0~12，其�?0代表总，其他代表Tx
 * @param  engerypoint: 指向电量存储的数组，默�?6字节BCD码，最低字节代表�?3,4小数
 * @param  decimalpoint: 代表显示的电量显示几位小数，0~4
 * @param  plusminus: 代表�?��显示负号�?    plus代表不显示，minus代表显示
 * @param  displayhighzero: 高位�?��显零，具体参见HIGHZERO_TYPE枚举
 * @retval None
 */
extern void Display_Engery(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate,unsigned char* engerypoint,unsigned char decimalpoint,PLUS_MINUS plusminus,HIGHZERO_TYPE displayhighzero)
{
    unsigned char valuebuf[4];

    if((date>12)||(decimalpoint>4)||rate>19)    //月份超过12月，小数点超�?4位或者费率超�?19不支持，返回
    {
        return;
    }

    //根据相位和电量�?类和费率判断要填充的�?��提示内�?
    Fill_Engery_In_ChineseHintArea(phase,engerytype,date,rate);
    
    //根据电量数组和其它形参填充数字区内�?
    Adjust_DecimalpointOfValue(engerypoint,valuebuf,decimalpoint);
    Fill_Value_In_NumberArea(valuebuf,decimalpoint,plusminus,displayhighzero);
    
    if((engerytype == CombinedActivePowerEnergy)||(engerytype == PositiveActivePowerEnergy)||(engerytype == ReverseActivePowerEnergy))     //有功�?
    {
        //�?��Kwh到单位区
        Fill_Kwh_In_UnitArea();
    }
    else
    {
        //�?��kvah到单位区
        Fill_kvah_In_UnitArea();        
    }


    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
    Refresh_NumberArea_of_LCD_DDRAM();              //刷新到LCD的数字提示区
    Refresh_UnitArea_of_LCD_DDRAM();                //刷新到LCD的单位区
}


/** 
 * @brief  显示当前日期
 * @note   
 * @param  datepoint: 指向日期存储的数组，默�?3字节BCD码，最低字节代表日
 * @retval None
 */
extern void Display_CurrentDate(unsigned char* datepoint)
{

    //�?��“当前日期”到�?��提示内�?
    Fill_CurrentDate_In_ChineseHintArea();
    //根据日期数组�?��数字区内�?
    Fill_Date_In_NumberArea(datepoint);

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
    Refresh_NumberArea_of_LCD_DDRAM();              //刷新到LCD的数字提示区
    Refresh_UnitArea_of_LCD_DDRAM();                //刷新到LCD的单位区(不能漏，�?��单位区清�?)
}

/** 
 * @brief  显示当前时间
 * @note   
 * @param  timepoint: 指向时间存储的数组，默�?3字节BCD码，最低字节代表�?
 * @retval None
 */
extern void Display_CurrentTime(unsigned char* timepoint)
{
    //�?��“当前时间”到�?��提示内�?
    Fill_CurrentTime_In_ChineseHintArea();
    //根据时间数组�?��数字区内�?
    Fill_Time_In_NumberArea(timepoint);

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
    Refresh_NumberArea_of_LCD_DDRAM();              //刷新到LCD的数字提示区
    Refresh_UnitArea_of_LCD_DDRAM();                //刷新到LCD的单位区(不能漏，�?��单位区清�?)
}


/** 
 * @brief  显示当前剩余金�?
 * @note   三相显示“当前剩余电费�? 单相显示“当前剩余金额�?
 * @param  amountpoint: 指向剩余金�?存储的数组，默�?4字节BCD码，最低字节代表小�?1�?2�?
 * @param  displayhighzero: 高位�?��显零�?0代表不显示， 1代表显示
 * @retval None
 */
extern void Display_RemainingAmount(unsigned char* amountpoint,unsigned char displayhighzero)
{
    //�?��“当前剩余金额”或者“当前剩余电费”到�?��提示内�?
    Fill_RemainingAmount_In_ChineseHintArea();
    //根据剩余金�?数组和其它参数填充数字区内�?
    Fill_Amount_In_NumberArea(amountpoint,Plus,displayhighzero);
    //�?��“元”到单位�?
    Fill_Yuan_In_UnitArea();

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
    Refresh_NumberArea_of_LCD_DDRAM();              //刷新到LCD的数字提示区
    Refresh_UnitArea_of_LCD_DDRAM();                //刷新到LCD的单位区(不能漏，�?��单位区清�?)
}

/** 
 * @brief  显示当前透支金�?
 * @note   三相显示“当前透支电费�? 单相显示“当前透支金�?�?
 * @param  amountpoint: 指向透支金�?存储的数组，默�?4字节BCD码，最低字节代表小�?1�?2�?
 * @param  displayhighzero: 高位�?��显零�?0代表不显示， 1代表显示
 * @retval None
 */
extern void Display_OverdraftAmount(unsigned char* amountpoint,unsigned char displayhighzero)
{
    //�?��“当前透支金�?”或者“当前透支电费”到�?��提示内�?
    Fill_OverdraftAmount_In_ChineseHintArea();
    //根据透支金�?数组和其它参数填充数字区内�?
    Fill_Amount_In_NumberArea(amountpoint,Minus,displayhighzero);
    //�?��“元”到单位�?
    Fill_Yuan_In_UnitArea();

    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
    Refresh_NumberArea_of_LCD_DDRAM();              //刷新到LCD的数字提示区
    Refresh_UnitArea_of_LCD_DDRAM();                //刷新到LCD的单位区(不能漏，�?��单位区清�?)    
}

/*end-------------------------------------------------------------------------*/
