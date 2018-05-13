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
#include "string.h"


/*宏定义----------------------------------------------------------------------*/
///添加宏定义
#define ChineseHintAreaGBKBufSize 20                    //中文提示区GBK码缓存区尺寸
#define ChineseHintAreaLCDRAMBackupBufSegSize   320     //中文提示区备用点阵缓存区尺寸

/*内部变量声明----------------------------------------------------------------*/
///添加内部变量
static short ChineseHintAreaGBKBuf[ChineseHintAreaGBKBufSize];//中文提示区GBK码缓存区
static unsigned char ChineseHintAreaLCDRAMBackupBuf[ChineseHintAreaLCDRAMBackupBufSegSize][ChineseHintAreaEndPageCom-ChineseHintAreaStartPageCom];  //中文提示区备用点阵缓存区

static unsigned char  ChineseHintArea_RollDisplay;      //中文提示区的名称是否启用动态显示标志，0不启用，1是代表启用，注意，初始化时默认启用，每次填中文提示区时先清零 */
/*内部变量初始化--------------------------------------------------------------*/
///添加内部变量
static 
/*声明内部函数----------------------------------------------------------------*/



//操作中文提示区GBK码缓存区的函数
static void Clear_ChineseHintAreaGBKBuf(void);
static void Wirte_ChineseHintAreaGBKBuf(short gbkcode);
static short Read_ChineseHintAreaGBKBuf(unsigned char index);
static unsigned char Read_GBKNum_ChineseHintAreaGBKBuf(void);
static void StrToChineseHintAreaGBKBuf(char *str);

//操作中文提示区备用点阵缓存区尺寸
static void Clear_ChineseHintAreaLCDRAMBackupBuf(void);
static void ChineseHintAreaLCDRAMBackupBuf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit);
static void InputCharacter_to_ChineseHintAreaLCDRAMBackupBuf(unsigned short x,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear);
static unsigned short ChineseHintAreaGBKBufToChineseBackupBuf(void);

static void Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf(unsigned short endseg,unsigned short offset);

static void Fill_Char_In_ChineseHintArea(unsigned char* strbuf,char SplitScreen);
/*定义内部函数----------------------------------------------------------------*/
///定义只能在本文件使用的函数


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
    ChineseHintArea_RollDisplay = 0;        //停止滚动显示
    Clear_ChineseHintAreaGBKBuf();          //清空
    Clear_ChineseHintAreaLCDRAMBackupBuf(); //清空

    //首先确定是否显示分屏，再确定中文提示区名称显示的范围
    //判断分屏是否显示
    if(SplitScreen < 0)                    //不显示分屏
    {
        endseg = ChineseHintAreaEndSeg;
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

        endseg = SplitWindowAreaStartseg;   //显示分屏了，中文提示区名称显示的范围就缩小了
    }

    StrToChineseHintAreaGBKBuf(strbuf);     //将字符串转成GBK码并放在ChineseHintAreaGBKBuf中
    len = ChineseHintAreaGBKBufToChineseBackupBuf();  //根据GBK码寻找到点阵数据并写到ChineseBackupBuf中
    Copy_ChineseHintArea_LCDRAM_BackupBuf_To_LCDRAM_Buf(endseg,0);  //根据中文提示区名称显示的范围从ChineseBackupBuf起始地址复制到ChineseHintArea_LCDRAM

    if(len > (endseg-ChineseHintAreaStartSeg))          //要显示的内容写不下
    {
        sprintf(tempstr,"%s%s",strbuf,Twospacebuf);     //在原先的字符串末尾添加2个空格，即“xx”变成“xx  ”
        StrToChineseHintAreaGBKBuf(tempstr);            //将字符串转成GBK码并放在ChineseHintAreaGBKBuf中
        len = ChineseHintAreaGBKBufToChineseBackupBuf();//根据GBK码寻找到点阵数据并写到ChineseBackupBuf中
        ChineseHintArea_RollDisplay = 1；               //启用滚动显示
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
    strcat(strbuf,"电量");
    //刷新LCD_RAM和backupbuf的点阵数据
    Fill_Char_In_ChineseHintArea(strbuf,-1);
}

/*定义全局函数----------------------------------------------------------------*/
///定义可用于外部文件的函数














/*end-------------------------------------------------------------------------*/