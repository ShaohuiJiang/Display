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
/*头文件----------------------------------------------------------------------*/
///添加头文件
#include "Display.h"
#include "CharLib.h"
#include "LCD.h"
#include "string.h"


/*宏定义----------------------------------------------------------------------*/
///添加宏定义
#define ChineseHintAreaGBKBufSize 20                    //中文提示区GBK码缓存区尺寸
#define ChineseHintAreaLCDRAMBackupBufSegSize   320     //中文提示区备用点阵缓存区尺寸

/*内部变量声明----------------------------------------------------------------*/
///添加内部变量
static short ChineseHintAreaGBKBuf[ChineseHintAreaGBKBufSize];  //中文提示区GBK码缓存区
static unsigned char ChineseHintAreaLCDRAMBackupBuf[ChineseHintAreaLCDRAMBackupBufSegSize][ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom];  //中文提示区备用点阵缓存区

static unsigned char  ChineseHintArea_RollDisplay;              //中文提示区的名称是否启用动态显示标志，0不启用，1是代表启用，注意，初始化时默认启用，每次填中文提示区时先清零 
static unsigned char  ChineseHintArea_SplitScreenDisplay;       //中文提示区的分屏是否启用标志，0不启用，1代表启用，注意，初始化时默认启用，每次填中文提示区时先清零
/*内部变量初始化--------------------------------------------------------------*/
///添加内部变量
/*声明内部函数----------------------------------------------------------------*/



//操作中文提示区GBK码缓存区的函数
static void Clear_ChineseHintAreaGBKBuf(void);
static void Wirte_ChineseHintAreaGBKBuf(unsigned char index,short gbkcode);
static short Read_ChineseHintAreaGBKBuf(unsigned char index);
static unsigned char Read_GBKNum_ChineseHintAreaGBKBuf(void);
static void StrToChineseHintAreaGBKBuf(char *str);

//操作中文提示区备用点阵缓存区尺寸
static void Clear_ChineseHintAreaLCDRAMBackupBuf(void);
static void ChineseHintAreaLCDRAMBackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit);
static void InputCharacter_to_ChineseHintAreaLCDRAMBackupBuf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear);
static unsigned short Read_SEGNum_ChineseHintAreaLCDRAMBackupBuf(void);

const unsigned char* Get_CharBufAddress( short gbkcode);
static void ChineseHintAreaGBKBufToChineseBackupBuf(void);

static void Copy_ChineseHintAreaGBKBuf_To_LCDRAM_Buf(unsigned short endseg,unsigned short offset);

static void Fill_Char_In_ChineseHintArea( char* strbuf,signed char SplitScreen);
/*定义内部函数----------------------------------------------------------------*/
/** 
 * @brief  ChineseHintAreaGBKBuf清零
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
 * @brief  往ChineseHintAreaGBKBuf指定位置写gbk码
 * @note   超范围就什么都不会写
 * @param  gbkcode: gbk码
 * @retval None
 */
static void Wirte_ChineseHintAreaGBKBuf(unsigned char index,short gbkcode)
{
    if(index>=ChineseHintAreaGBKBufSize)
    {
        return ;
    }
    ChineseHintAreaGBKBuf[index] = gbkcode;
}
/** 
 * @brief  读取ChineseHintAreaGBKBuf指定位置的gbk码
 * @note   超范围就返回0
 * @param  index:  索引
 * @retval 0：无效gbk码，其他值：对应的gbk码
 */
static short Read_ChineseHintAreaGBKBuf(unsigned char index)
{
    if(index>=ChineseHintAreaGBKBufSize)
    {
        return 0;
    }  
    return ChineseHintAreaGBKBuf[index];
}
/** 
 * @brief  ChineseHintAreaGBKBuf中非0的GBK码数量
 * @note   
 * @retval 数量
 */
static unsigned char Read_GBKNum_ChineseHintAreaGBKBuf(void)
{
    unsigned char index = 0;
    short value;
    for(index=0;index<ChineseHintAreaGBKBufSize;index++)
    {
        value = Read_ChineseHintAreaGBKBuf(index);
        if(value == 0)
        {
            return index;
        }
    }
    return index;
}
/** 
 * @brief  将字符串转成short型的GBK码写到ChineseHintAreaGBKBuf中
 * @note   
 * @param  *str: 字符串
 * @retval None
 */
static void StrToChineseHintAreaGBKBuf(char *str)
{
    unsigned char offset;
    unsigned char i;
    int len;
    short tempgbk;
    
    Clear_ChineseHintAreaGBKBuf();                  //清空
    len = strlen(str);
    offset = 0;
    for(i=0;i<len;i++)
    {
        if(*(str+offset) == 0)
        {
            return;
        }
        else if(*(str+offset)<0x80)       //字母或者数字
        {
            tempgbk = (unsigned short)*(str+offset);
            Wirte_ChineseHintAreaGBKBuf(i,tempgbk);
            offset++;
        }
        else if(*(str+offset)>0x80)      //汉字
        {
            tempgbk = (unsigned short)*(str+offset)*256+(unsigned short)*(str+offset+1);
            Wirte_ChineseHintAreaGBKBuf(i,tempgbk);
            offset +=2;
        }
    }


}

/** 
 * @brief  ChineseHintAreaLCDRAMBackupBuf清零
 * @note   
 * @retval None
 */
static void Clear_ChineseHintAreaLCDRAMBackupBuf(void)
{
    unsigned short i;
    unsigned char j;
    for(i=0;i<ChineseHintAreaLCDRAMBackupBufSegSize;i++)
    {
        for(j=0;j<(ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom);j++)
        {
            ChineseHintAreaLCDRAMBackupBuf[i][j] = 0;
        }
    }        
}
/** 
 * @brief  ChineseHintAreaLCDRAMBackupBuf描点函数
 * @note   超范围的点，什么都不写
 * @param  x: seg点
 * @param  y: com点
 * @param  bit: 0：写0,1：写1
 * @retval None
 */
static void ChineseHintAreaLCDRAMBackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit)
{
    unsigned short pos;
    unsigned short bx;
    unsigned char temp=0;

    if((x>=(ChineseHintAreaLCDRAMBackupBufSegSize)||y>=(ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom)*8))
    {
        return;       //超出范围
    }
    pos=y/8;          //得到页地址       
    
    bx=y%8;           //得到点在页地址的bit位
    temp=1<<(7-bx);
    if(bit)
    {
        ChineseHintAreaLCDRAMBackupBuf[x][pos]|=temp;
    }
    else 
    {
        ChineseHintAreaLCDRAMBackupBuf[x][pos]&=~temp; 
    }   
}
/** 
 * @brief  将字符的点阵信息写到ChineseHintAreaLCDRAMBackupBuf指定位置的
 * @note   
 * @param  x: seg位置
 * @param  y: com位置
 * @param  charbufstartaddress: 字符点阵信息
 * @param  size: 尺寸
 * @param  displayorclear: 显示或者清除
 * @retval None
 */
static void InputCharacter_to_ChineseHintAreaLCDRAMBackupBuf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear)
{
    unsigned char temp;
    unsigned short y0;
    unsigned int t,t1,segnumber,comnumber,bytesnumber; 

    y0 = y;
    segnumber = size/100;
    comnumber = size%100;
    bytesnumber = segnumber*(comnumber/8+((comnumber%8)?1:0));      //得到需要显示所用的字节数
    
    for(t=0;t<bytesnumber;t++)                                      //将显示所用的字节全部写到ChineseHintAreaLCDRAMBackupBuf数组中
    {     
        temp=*(charbufstartaddress+t);
        for(t1=0;t1<8;t1++) 
        {
            if(temp&0x80)
            {
                if(displayorclear == display)      //写1，即显示
                {
                    ChineseHintAreaLCDRAMBackupBuf_DrawPoint(x,y,1);
                }
                else                              //写0，即清除
                {
                    ChineseHintAreaLCDRAMBackupBuf_DrawPoint(x,y,0);
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
 * @brief  得到ChineseHintAreaLCDRAMBackupBuf的有效列数
 * @note   注意默认为从最大开始倒数，碰到的非零结束
 * @retval 有效的列数
 */
static unsigned short Read_SEGNum_ChineseHintAreaLCDRAMBackupBuf(void)
{
    unsigned short i;
    unsigned short len1;
    unsigned short len2;
    short gbk;
    unsigned int sizenumber;
    unsigned int size;

    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    sizenumber = Size_6P12P;
    #else
    size = Size_14P14P;
    sizenumber = Size_7P14P;
    #endif

    len1 = Read_GBKNum_ChineseHintAreaGBKBuf();
    len2 = 0; 
    for(i=0;i<len1;i++)
    {  
        gbk = Read_ChineseHintAreaGBKBuf(i);
        if((gbk&0x8080) == 0x8080)
        {
            len2 += size/100;
        }
        else
        {
            len2 += sizenumber/100;
        }
    }

    return len2;
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
    short temp;

    for(i=0;i<GBKNumber;i++)
    {
         temp = charaddressbuf[i].CharGbk;
        if(gbkcode == temp)
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
 * @brief  将ChineseHintAreaGBKBuf中的gbk码转化成点阵信息存到ChineseBackupBuf
 * @note   
 * @retval None
 */
static void ChineseHintAreaGBKBufToChineseBackupBuf(void)
{
    unsigned char i;
    unsigned char len;
    short tempgbk;
    const unsigned char* address;
    unsigned int sizenumber;
    unsigned int size;
    unsigned short offset;

    Clear_ChineseHintAreaLCDRAMBackupBuf();         //清空
    #if (MeterType == ThreePhaseMeter)
    size = Size_12P12P;
    sizenumber = Size_6P12P;
    #else
    size = Size_14P14P;
    sizenumber = Size_7P14P;
    #endif

    offset = 0;
    len = Read_GBKNum_ChineseHintAreaGBKBuf();
    for(i=0;i<len;i++)
    {
        tempgbk = Read_ChineseHintAreaGBKBuf(i);
        address = Get_CharBufAddress(tempgbk);
        if((tempgbk&0x8080) == 0x8080)  //汉字
        {
            InputCharacter_to_ChineseHintAreaLCDRAMBackupBuf(offset,0,address,size,display);
            offset += size/100;
        }
        else                         //数字字母
        {
            InputCharacter_to_ChineseHintAreaLCDRAMBackupBuf(offset,0,address,sizenumber,display);
            offset += sizenumber/100;
        }
    }
}

/** 
 * @brief  将ChineseHintAreaGBKBuf有效内容复制到LCDRAM_Buf
 * @note   
 * @param  chinesehintareaLCDRAMendseg: LCDRAM_Buf中的中文提示区名称的边界
 * @param  offset: ChineseHintAreaGBKBuf起始地址的偏移
 * @retval None
 */
static void Copy_ChineseHintAreaGBKBuf_To_LCDRAM_Buf(unsigned short chinesehintareaLCDRAMendseg,unsigned short offset)
{
    unsigned short i,j,len;
    unsigned short index;
    len = Read_SEGNum_ChineseHintAreaLCDRAMBackupBuf();
    if(offset>len)
    {
        return;
    }
    if(len<=(chinesehintareaLCDRAMendseg-ChineseHintAreaStartSeg))  //ChineseHintAreaGBKBuf有效的边界小于等于中文提示区名称的边界
    {
        index = offset;
        for(i=0;i<(len-offset);i++)
        {
            index++;
            for(j=0;j<(ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom);j++)
            {
                Wirte_LCDRAM_Buf(ChineseHintAreaStartSeg+i,ChineseHintAreaStartPageCom+j,ChineseHintAreaLCDRAMBackupBuf[index][j]);
            }
        }  
    }
    else 
    {
        index = offset;
        for(i=0;i<(chinesehintareaLCDRAMendseg-ChineseHintAreaStartSeg);i++)
        {   
            index++;
            if(index >= len)
            {
                index = 0;
            }
            for(j=0;j<(ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom);j++)
            {
                Wirte_LCDRAM_Buf(ChineseHintAreaStartSeg+i,ChineseHintAreaStartPageCom+j,ChineseHintAreaLCDRAMBackupBuf[index][j]);
            }
        }
        len = Read_SEGNum_ChineseHintAreaLCDRAMBackupBuf();
    }
}
static void Fill_Char_In_ChineseHintArea( char* strbuf, signed char SplitScreen)
{
 
    unsigned short endseg;
    unsigned short len;
    const unsigned char* addresspoint;
    unsigned int size;

    ChineseHintArea_SplitScreenDisplay = 0; //默认为无分屏
    ChineseHintArea_RollDisplay = 0;        //停止滚动显示
    Clear_ChineseHintAreaGBKBuf();          //清空
    Clear_ChineseHintAreaLCDRAMBackupBuf(); //清空

    //首先确定是否显示分屏，再确定中文提示区名称显示的范围
    //判断分屏是否显示
    if(SplitScreen < 0)                    //不显示分屏
    {
        endseg = ChineseHintAreaEndSeg;
        ChineseHintArea_SplitScreenDisplay = 0;
    }
    else if(SplitScreen < 9)              //显示分屏
    {
        #if (MeterType == ThreePhaseMeter)
        addresspoint = &SplitScreenIcon_13p12p[SplitScreen][0];
        size = Size_13P12P;
        #else
        addresspoint = &SplitScreenIcon_14p14p[SplitScreen][0];
        size = Size_14P14P;
        #endif
        InputCharacter_to_LCDRAM_Buf(SplitWindowAreaStartseg,SplitWindowAreaStartCom,addresspoint,size,display);
        endseg = SplitWindowAreaStartseg-1;       //显示分屏了，中文提示区名称显示的范围就缩小了
        ChineseHintArea_SplitScreenDisplay = 1; //显示分屏标志位置1
    }

    StrToChineseHintAreaGBKBuf(strbuf);         //将字符串转成GBK码并放在ChineseHintAreaGBKBuf中
    ChineseHintAreaGBKBufToChineseBackupBuf();  //根据GBK码寻找到点阵数据并写到ChineseBackupBuf中
    len = Read_SEGNum_ChineseHintAreaLCDRAMBackupBuf();
    Copy_ChineseHintAreaGBKBuf_To_LCDRAM_Buf(endseg,0);  //根据中文提示区名称显示的范围从ChineseBackupBuf起始地址复制到ChineseHintArea_LCDRAM

    if(len > (endseg-ChineseHintAreaStartSeg))          //要显示的内容写不下
    {
        strcat(strbuf," ");                            //末尾增加1个空格，作为隔离

        StrToChineseHintAreaGBKBuf(strbuf);             //将字符串转成GBK码并放在ChineseHintAreaGBKBuf中      
        ChineseHintAreaGBKBufToChineseBackupBuf();      //根据GBK码寻找到点阵数据并写到ChineseBackupBuf中
        len = Read_SEGNum_ChineseHintAreaLCDRAMBackupBuf();
        ChineseHintArea_RollDisplay = 1;                //启用滚动显示
    }
}

/** 
 * @brief  将电量类名称写到中文提示区
 * @note   
 * @param  phase: 相位
 * @param  engerytype: 电量类型
 * @param  date: 时间
 * @param  rate: 费率
 * @retval None
 */
static void Fill_Engery_In_ChineseHintArea(PHASE_TYPE phase,ENERGY_TYPE engerytype,unsigned char date,unsigned char rate)
{
    char strbuf[100] = "";
    char strbuf2[20] = "";
    int num;

    if((date>99)||rate>99)    //月份超过99月，小数点超过4位或者费率超过99不支持，返回
    {
        return;
    }
    //确定时间
    if(date == 0)       //当前
    {
        strcat(strbuf,"当前");
    }
    else                //上x月
    {
        strcat(strbuf,"上");
        num  = (int)date;
        strcat(strbuf2,"月");
        sprintf(strbuf,"%s%d%s",strbuf,num,strbuf2);
    }

    //确定相位
    switch(phase)
    {
        case TotalPhase:
            strcat(strbuf,"");
        break;
        case APhase:
            strcat(strbuf,"A相");
        break;
        case BPhase:
            strcat(strbuf,"B相");
        break;
        case CPhase:
            strcat(strbuf,"C相");
        break;
        default:
        break;
    }
    //确定电量类型
    switch(engerytype)
    {
        case CombinedActivePowerEnergy:
            #if (MeterType == ThreePhaseMeter)
            strcat(strbuf,"组合有功");
            #else
            strcat(strbuf,"有功");
            #endif
        break;
        case PositiveActivePowerEnergy:
            strcat(strbuf,"正向有功");
        break;
        case ReverseActivePowerEnergy:
            strcat(strbuf,"反向有功电量");
        break;
        case CombinedOneReactivePowerEnergy:
            strcat(strbuf,"组合无功1");
        break;
        case CombinedTwoReactivePowerEnergy:
            strcat(strbuf,"组合无功2");
        break;
        case ReverseReactivePowerEnergy:
            strcat(strbuf,"反向无功");
        break;
        case FirstQuadrantReactivePowerEnergy:
            strcat(strbuf,"第1象限无功");
        break;
        case SecondQuadrantReactivePowerEnergy:
            strcat(strbuf,"第2象限无功");
        break;
        case ThirdQuadrantReactivePowerEnergy:
            strcat(strbuf,"第3象限无功");
        break;
        case FourthQuadrantReactivePowerEnergy:
            strcat(strbuf,"第4象限无功");
        break;
        case PositiveApparentEnergy:
            strcat(strbuf,"正向视在");
        break;
        case ReverseApparentEnergy:
            strcat(strbuf,"反向视在");
        break;
        default:
            strcat(strbuf,"");
        break;
    }
    //确定费率
    if(rate == 0)
    {
        strcat(strbuf,"总");
    }
    else
    {
        strcat(strbuf,"T");
        num = (int)rate;
        sprintf(strbuf,"%s%d",strbuf,num);
    }

    //最后写电量二字
    strcat(strbuf,"电量量");
    //刷新LCD_RAM和backupbuf的点阵数据
    Fill_Char_In_ChineseHintArea(strbuf,1);
}

/*定义全局函数----------------------------------------------------------------*/
extern void RollDiplay_ChineseHintArea(void)
{
    static short offset;
    unsigned short endseg;
    unsigned short len;
    
    if(ChineseHintArea_RollDisplay == 1)
    {
        len = Read_SEGNum_ChineseHintAreaLCDRAMBackupBuf();
        if(ChineseHintArea_SplitScreenDisplay == 1)     //存在分屏
        {
            endseg = SplitWindowAreaStartseg-1;
        }
        else                                            //不存在分屏
        {
            endseg = ChineseHintAreaEndSeg;
        }
        if(offset >= (len-1))
        {
            offset = offset;
        }
        Copy_ChineseHintAreaGBKBuf_To_LCDRAM_Buf(endseg,offset);  //根据中文提示区名称显示的范围从ChineseBackupBuf起始地址复制到ChineseHintArea_LCDRAM
        
        offset++;
        
        if(offset >= len)
        {
            offset = 0;
        }
    }
    else
    {
        offset = 0;
    }

    Refresh_ChineseHintArea_of_LCD_DDRAM();
}
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
    if((date>99)||(decimalpoint>4)||rate>99)    //月份超过99月，小数点超过4位或者费率超过99不支持，返回
    {
        return;
    }

    Fill_Engery_In_ChineseHintArea(phase,engerytype,date,rate);



    if((engerytype == CombinedActivePowerEnergy)||(engerytype == PositiveActivePowerEnergy)||(engerytype == ReverseActivePowerEnergy))     //有功类
    {
        //填充Kwh到单位区
      //  Fill_Kwh_In_UnitArea();
    }
    else
    {
        //填充kvah到单位区
      //  Fill_kvah_In_UnitArea();        
    }


    Refresh_ChineseHintArea_of_LCD_DDRAM();         //刷新到LCD的中文提示区
}


extern void Display_QuadrantStatus(unsigned char number)
{
    if((number==0)||(number>4))
    {
        number =1;
    }

    Clear_QuadrantArea_Of_LCDRAM_Buf();
    InputCharacter_to_LCDRAM_Buf(QuadrantAreaStartSeg,QuadrantAreaStartPageCom,&QuadrantIcon_12p12p[number-1][0],Size_12P12P,display);
    Refresh_QuadrantArea_of_LCD_DDRAM();
}










/*end-------------------------------------------------------------------------*/
