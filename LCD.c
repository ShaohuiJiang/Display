/*******************************************************************************
* @file      : LCD.c
* @author    : Jiangsh
* @version   : V1.0.0
* @date      : Thu Apr 12 2018
* @brief     : 
********************************************************************************
* @attention :如果本文本中存在汉字变量，文本要转化成GB2312编码格式保存和打开
*
*
*/
/*头文件----------------------------------------------------------------------*/
///添加头文件
#include "LCD.h" 
#include "CharLib.h"
#include "LCDConfig.h"
/*宏定义----------------------------------------------------------------------*/

/*内部变量声明----------------------------------------------------------------*/
/* LCD的缓存数组 */
static unsigned char LCDRAM_Buf[seg][Page];

/*声明内部函数----------------------------------------------------------------*/
static void Delay_Nms(unsigned int n);
static void SendByte(unsigned char Dbyte);
static void Writecom(unsigned char wcom);
static void Writedata(unsigned char wdata);
 //该函数没有用到，先屏蔽，避免编译器告警 static unsigned char Readcom(void);
 //该函数没有用到，先屏蔽，避免编译器告警 static void Clear_ICON_DDRAM(void);
 //该函数没有用到，先屏蔽，避免编译器告警 static void OTP_Write(void);
static void window(void);
static void set_window_area(unsigned short startseg,unsigned short startpage,unsigned short endseg,unsigned short endpage);
static void LCDRAM_Buf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit);
/*定义内部函数----------------------------------------------------------------*/

 /** 
  * @brief  毫秒级延时
  * @note   
  * @param  n: 0~65565
  * @retval None
  */
static void Delay_Nms(unsigned int n) 
{
    unsigned int i,j;
    for(i=0;i<n;i++)
    for(j=0;j<1000;j++);
}

/*内部：LCD通信函数------------------------*/
/** 
 * @brief  向LCD发送一个字节
 * @note   字节可能是命令也可能是数据，看使用环境
 * @param  Dbyte: 一个字节
 * @retval None
 */
static void SendByte(unsigned char Dbyte)
{
    unsigned char i,j;
    
    for(i=0 ;i< 8 ;i++)
    {
        SCK_Low();
        
        j=0X80&Dbyte;
       
        Dbyte=Dbyte<< 1;   
        
        if(j&0x80)
        {
            SDA_High();
        }
        else
            SDA_Low();      //移出的位给SDA
        
        SCK_High();	
        SCK_Low();	
    }
}

/** 
 * @brief  向LCD发送一个字节命令
 * @note   
 * @param  wcom: 一个字节命令
 * @retval None
 */
static void Writecom(unsigned char wcom)
{
    CS_Low();
    A0_Low();
    SendByte(wcom);
    CS_High();
}

/** 
 * @brief  向LCD发送一个字节数据
 * @note   
 * @param  wdata: 一个字节数据
 * @retval None
 */
static void Writedata(unsigned char wdata)
{
    CS_Low();
    A0_High();
    SendByte(wdata);
    CS_High();
}

/** 
 * @brief  从LCD读一个字节数据
 * @note   
 * @retval 读到的数据
 */
/*该函数没有用到，先屏蔽，避免编译器告警
static unsigned char Readcom(void)
{	
    unsigned char i,L0;
	L0=0;
	Writecom(0x38);     	
    Writecom(0x7F);     	//使能读ID
	Writecom(0x30);     	

	CS_Low();
    A0_Low();
    SendByte(0xFE) ;
    A0_High();
  
   	CS_Low();
	SDA_High();
  	A0_High();
	
	SDA_IN();
    for(i=0;i<8;i++)
    {
  		SCK_Low();
  		if(SDA_STATUS() == (1<<6))
  		L0=L0+1;
  	
  		SCK_High();
  	    if(i<7)
   	    L0=L0<<1;
    }
    CS_High() ;
	Writecom(0x38);     	
    Writecom(0x7E);     	
	Writecom(0x30);     

    return L0;	
}
*/
/** 
 * @brief  清除LCD中ICON RAM区的内容，并禁止ICON RAM
 * @note   
 * @retval None
 */
 /*该函数没有用到，先屏蔽，避免编译器告警
static void Clear_ICON_DDRAM(void)
{
    unsigned char j;
    Writecom(0x30);	    //Extension Command 1
    Writecom(0x77);	    //Enable ICON RAM

    Writecom(0x15);    	//Column Address Setting 
    Writedata(0x00);    //Starting column address 
    Writedata(0x9F);    //Ending column address
    Writecom(0x5c); 	//Write data
    for(j=0;j<seg;j++)
    {
        Writedata(0x00);
    }
    Writecom(0x76);	    //Disable ICON RAM  
}
*/
/** 
 * @brief  执行以下命令，烧OTP和ID
 * @note   具体作用也不是很清楚，估计要问液晶厂家
 * @retval None
 */
 /*该函数没有用到，先屏蔽，避免编译器告警
static void OTP_Write(void)   
{    
    Writecom(0x30);  // Extension Command 1 
    Writecom(0xAE);  // Display OFF 
    Delay_Nms(50);  // Delay 50ms 
    Writecom(0x39);  //Enable OTP 
    Writecom(0XD6);   
    Writedata(0x10);   
    
    Writecom(0x31);  // Extension Command 2 
    Writecom(0XF0);  // Frame Rate = 73Hz 
    Writedata(0x0D);   
    Writedata(0x0D);   
    Writedata(0x0D);   
    Writedata(0X0D);   
    
    Writecom(0xE4);  // OTP Selection Control 
    Writedata(0x99);   
    Writecom(0xE5);  // OTP Programming Setting 
    Writedata(0x0F);   
    Writecom(0xE0);  // OTP WR/RD Control 
    Writedata(0x20);   
    Delay_Nms(100);  // Delay 100ms 
    Writecom(0xE2);  // OTP Write 
    Delay_Nms(100);  // Delay 100ms 
    Writecom(0xE1);  // OTP Control Out 
    Writecom(0x30);  // Extension Command 1 
} 
*/
/*内部：设置LCD芯片显示区刷新范围函数------------------------*/
/** 
 * @brief  定义LCD的所有DDRAM区域都可写
 * @note   
 * @retval None
 */
static void window(void)
{
 	Writecom(0x30);    	//Extension Command 1 
	Writecom(0x15);    	//Column Address Setting 
	Writedata(0x00);    //Starting column address
	Writedata(0x9F);    //Ending column address

	Writecom(0x75);    	//Row Address Setting 
	Writedata(0x00);    //Starting page address 
	Writedata(0x13); 	//Ending page address
	Writecom(0x5c); 	//Write data
}

/** 
 * @brief  设置可写DDRAM区域
 * @note   通过这个函数对应往LCD写的第一个数据是对应液晶的哪里，另外范围是前闭后开
 * @param  startseg: 起始列地址
 * @param  startpage: 起始页地址
 * @param  endseg: 结束列地址
 * @param  endpage: 结束页地址
 * @retval None
 */
static void set_window_area(unsigned short startseg,unsigned short startpage,unsigned short endseg,unsigned short endpage)
{
 	Writecom(0x30);    	                        //Extension Command 1 
	Writecom(0x15);    	                        //Column Address Setting 
	Writedata((unsigned char)startseg);         //Starting column address
	Writedata(((unsigned char)endseg-1));       //Ending column address

	Writecom(0x75);    	                        //Row Address Setting 
	Writedata((unsigned char)startpage);        //Starting page address 
	Writedata(((unsigned char)endpage-1)); 	    //Ending page address
	Writecom(0x5c); 	                        //Write data    
}



/*内部：操作LCDRAM_Buf数组函数------------------------*/
/** 
 * @brief  LCDRAM_Buf任意位置描点函数
 * @note   (x,y)位置的某点，显示或者不显示，
 * 注意，这个只是描了缓存区的点，最终显示需要调用其它函数将缓存区写到LCD
 * @param  x: seg的位置
 * @param  y: com的位置
 * @param  bit: 0：不显示，1：显示
 * @retval None
 */
static void LCDRAM_Buf_DrawPoint(unsigned short x,unsigned short y,unsigned char bit)
{
    unsigned short pos;
    unsigned short bx;
    unsigned char temp=0;

    if(x>=seg||y>=com)return;       //超出范围
    pos=y/8;        //得到页地址       
    
    bx=y%8;         //得到点在页地址的bit位
    temp=1<<(7-bx);

    if(bit)
    {
        LCDRAM_Buf[x][pos]|=temp;
    }
    else 
    {
        LCDRAM_Buf[x][pos]&=~temp; 
    }   
}


/*定义全局函数----------------------------------------------------------------*/

/*LCD硬件相关函数---------------------------*/
/** 
 * @brief  LCD配置初始化
 * @note   该函数调用前，确保LCD操控管脚已经配置到位
 * @retval None
 */
extern void LCD_Config_Init(void)
{
	int Data = 0;
    
    ///复位LCD驱动芯片
    RST_Low();
    Delay_Nms(20);
    RST_High();
    Delay_Nms(20);
 
    ///扩展命令2里的命令设置
    Writecom(0x31); // Extension Command 2
    ////OTP自动读取的状态关闭？？
    Writecom(0xD7); // Disable Auto Read
    Writedata(0x9F);
    ////使能OTP读状态？？
    Writecom(0xE0); // Enable OTP Read
    Writedata(0x00);
    Delay_Nms(20);
    
    ////触发OTP编程过程？？
    Writecom(0xE3); // OTP Up-Load
    Delay_Nms(20);
    ////取消OTP控制
    Writecom(0xE1); // OTP Control Out
    
    ///扩展命令1的命令设置
    Writecom(0x30); // Extension Command 1
    ////退出休眠状态
    Writecom(0x94); // Sleep Out
    ////关闭显示
    Writecom(0xAE); // Display OFF
    Delay_Nms(50);
    ////打开升压电路
    Writecom(0x20); //Power Control
    Writedata(0x0B); // VB, VR,VF All ON
    Delay_Nms(100);
    ////设置Vop电压值
    Writecom(0x81); // Set Vop = 14.0V
    Writedata(0x04);
    Writedata(0x04);

    // Frame Rate（帧率）
    ///扩展命令2里的命令设置
    Writecom(0x31);   // Extension Command 2
    ////设置不同温度下的帧率
    Writecom(0XF0);   // Frame Rate
    Writedata(0x02);  // 34.5Hz  //低温-30  -40效果最佳（-40~TA）
    Writedata(0x06);  // 46(TA~TB)
    Writedata(0x14);  // 83(TB~TC)
    Writedata(0x1c);  // 138(TC~90)
    ////TA/TB/TC的温度点
    Writecom(0XF2);   // Temperature Range
    Writedata(0x06);  // -34C（TA）
    Writedata(0x1c);  // -12C (TB)
    Writedata(0x50);  //  40C (TC)
    
    //Temperature Gradient Compensation(温度梯度补偿系数)
    Writecom(0xf4);
    Writedata(0x10) ;  //低温-30  -40效果最佳 常温下 VOP=12.7V(C9两端)
    Writedata(0x11);  //低温-30  -40效果最佳
    Writedata(0x00);
    Writedata(0x00);
    Writedata(0x00);
    Writedata(0x11);
    Writedata(0x51);
    Writedata(0x35);

    ///扩展命令2里的命令设置
    Writecom(0x31); // Extension Command 2
    ////该指令用于设置增强器效率和偏置比。
    Writecom(0x32); // Analog Circuit Set
    Writedata(0x00);
    Writedata(0x01); // Booster Efficiency =Level1
    Writedata(0x04); //Bias=1/10
	////该指令用于控制内置升压电路以提供内置稳压器的电源					
    Writecom(0x51); // Booster Level x10
    Writedata(0xFB);
    
	///扩展命令2里的命令设置					
    Writecom(0x30); // Extension Command 1
    ///该指令定义显示模式为灰度模式或单色模式，定义为单色模式
    Writecom(0xF0); //Display Mode
    Writedata(0x10); //Mono Mode
    ///设置显示控制
    Writecom(0xCA); //Display Control
    Writedata(0x00); // CL Dividing Ratio => Not Divide//设置CL分割比例
    Writedata(0X4F); //Duty Set => 80 Duty//
    Writedata(0x00); //Frame Inversion
						
    Writecom(0xBC); // Data Scan Direction（设置扫描方向）
    Writedata(0x02);//00 与 02 扫描顺序//00是从左往右，从上往下，02是从右往左，从上往下
    Writecom(0xA6); // Normal Display//正常显示
            
    Writecom(0x31); //Extension Command 2
    Writecom(0x40); //Internal Power Supply（内部电源）
    Writecom(0x30); // Extension Command 1

    // icon clear（图标ram区清零）
    Writecom(0x77); // Enable ICON RAM
    Writecom(0x5C);
    for(Data = 0; Data <= 255; Data++)
    {
        Writedata(0);
    }
    
    Writecom(0x76); // Disable ICON RAM（禁止图标ram）

    Writecom(0x30); //Extension Command 1
    ////设置可访问的列地址的区域
    Writecom(0x15); // Column Address Setting
    Writedata(0x00); // SEG0 -> SEG159
    Writedata(0x9F);
    ////设置可访问的页地址的区域
    Writecom(0x75); // Row Address Setting
    Writedata(0x00); // COM0 -> COM59
    Writedata(0x13);
    
    //清空自己定义的缓存区，也清空LCD的DDRAM
    Clear_LCDRAM_Buf();
    Refresh_LCD_DDRAM();
    Writecom(0xAF); // Display ON（打开显示）
}
/** 
 * @brief  设置ID
 * @note   ID用来干什么不清楚，估计要问液晶厂家
 * @retval None
 */
extern void Set_OTP_Register(void)   
{   
    Writecom(0x38);  // Extension Command 3 
    Writecom(0xD5);  // Set ID=3
    Writedata(0x03);   
}

/** 
 * @brief  LCD的总初始化函数
 * @note   集合了管脚初始化、配置初始化和设置ID
 * @retval None
 */
extern void LCD_Init(void)
{
    LCD_GPIO_Init();
    LCD_Config_Init();  
    Set_OTP_Register();       
}

extern void Closed_LCD(void)
{
    ///扩展命令1的命令设置
    Writecom(0x30); // Extension Command 1
    ////关闭显示
    Writecom(0xAE); // Display OFF
    ////退出休眠状态
    Writecom(0x95); // Sleep IN
    Delay_Nms(100);
    ///置低LCD驱动芯片复位脚
    RST_Low();
}




/*操作LCDRAM_Buf数组函数------------------------*/

/** 
 * @brief  往LCDRAM_Buf的指定位置显示或清除一个字符(包括汉字、字母、数字、符号)
 * @note   描点方式是逐列式，清除的话，确保同一位置显示了对应字符，不然不保证清除效果
 * @param  x: 指定起始位置的seg
 * @param  y: 指定起始位置的com
 * @param  charbufstartaddress: 字符的字模起始地址
 * @param  size: 字符尺寸
 * @param  displayorclear: clear代表清除，display代表显示
 * @retval None
 */
extern void InputCharacter_to_LCDRAM_Buf(unsigned short x,unsigned short y,const unsigned char* charbufstartaddress,unsigned int size,unsigned char displayorclear)
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

                if((x>=seg)||(y>=com))  //超范围了
                {
                    return;             //超出数组范围的，不写
                }

                if(displayorclear == display)      //写1，即显示
                {
                    LCDRAM_Buf_DrawPoint(x,y,1);
                }
                else                                //写0，即清除
                {
                    LCDRAM_Buf_DrawPoint(x,y,0);
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
 * @brief  清除LCDRAM_Buf数组中指定范围内的数据
 * @note   注意，范围采用前闭后开方式，即起始点会被清除，结束点不会被清除
 * @param  x1: 起始seg，0~(seg-1)
 * @param  y1: 起始com 0~(com-1)
 * @param  x2: 结束seg 1~seg
 * @param  y2: 结束com 1~com
 * @retval None
 */
extern void Clear_PartArea_of_LCDRAM_Buf(unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2)
{
    unsigned short j,k;
    if((x1>=seg)||(y1>=com)||(x2>seg)||(y2>com))//位置超范围了，直接返回
    {
        return;         
    }
    if((x1>=x2)||(y1>=y2))  //起始位置必须小于结束位置，否则直接返回
    {
        return;
    }

    for(j=y1;j<y2;j++)
    {
        for(k=x1;k<x2;k++)
        {
            LCDRAM_Buf_DrawPoint(k,j,0);  
        }       
    }
}

/** 
 * @brief  LCDRAM_Buf数组清零
 * @note   
 * @retval None
 */
extern void Clear_LCDRAM_Buf(void)
{ 
    Clear_PartArea_of_LCDRAM_Buf(0,0,seg,com);
}

#if (MeterType == ThreePhaseMeter)
/** 
 * @brief  清除LCDRAM_Buf数组的象限区的内容
 * @note   
 * @retval None
 */
extern void Clear_QuadrantArea_Of_LCDRAM_Buf(void)
{
    Clear_PartArea_of_LCDRAM_Buf(QuadrantAreaStartSeg,QuadrantAreaStartCom,QuadrantAreaEndSeg,QuadrantAreaEndCom);
}
#endif 

/** 
 * @brief  清除LCDRAM_Buf数组的中文提示区的内容
 * @note   
 * @retval None
 */
extern void Clear_ChineseHintArea_Of_LCDRAM_Buf(void)
{
    Clear_PartArea_of_LCDRAM_Buf(ChineseHintAreaStartSeg,ChineseHintAreaStartCom,ChineseHintAreaEndSeg,ChineseHintAreaEndCom);
}

/** 
 * @brief  清除LCDRAM_Buf数组的数字区的内容
 * @note   
 * @retval None
 */
extern void Clear_NumberArea_Of_LCDRAM_Buf(void)
{
    Clear_PartArea_of_LCDRAM_Buf(NumberAreaStartSeg,NumberAreaStartCom,NumberAreaEndSeg,NumberAreaEndCom);
}

/** 
 * @brief  清除LCDRAM_Buf数组的报警提示和单位区的内容
 * @note   
 * @retval None
 */
extern void Clear_AlarmHintArea_Of_LCDRAM_Buf(void)
{
    Clear_PartArea_of_LCDRAM_Buf(AlarmHintAreaStartSeg,AlarmHintAreaStartCom,AlarmHintAreaEndSeg,AlarmHintAreaEndCom);
}
extern void Clear_UnitArea_Of_LCDRAM_Buf(void)
{
    Clear_PartArea_of_LCDRAM_Buf(UnitAreaStartSeg,UnitAreaStartCom,UnitAreaEndSeg,UnitAreaEndCom);
}
/** 
 * @brief  清除LCDRAM_Buf数组的状态提示区的内容
 * @note   
 * @retval None
 */
extern void Clear_StatusHintArea_Of_LCDRAM_Buf(void)
{
    Clear_PartArea_of_LCDRAM_Buf(StatusHintAreaStartSeg,StatusHintAreaStartCom,StatusHintAreaEndSeg,StatusHintAreaEndCom);
}




/*刷新LCD芯片显示区函数------------------------*/
/** 
 * @brief  刷新LCD整屏的显示内容
 * @note   是以变量数组LCDRAM_Buf中的内容为准
 * @retval none
 */
extern void Refresh_LCD_DDRAM(void)
{  
    unsigned char i,j;
    window();
    for(i=0;i<Page;i++)         //页地址范围
    {
        for(j=0;j<seg;j++)      //段地址范围
        {
            Writedata(LCDRAM_Buf[j][i]);
        }
    }
}

#if (MeterType == ThreePhaseMeter)
/** 
 * @brief  刷新LCD的象限区显示内容
 * @note   是以变量数组LCDRAM_Buf中的内容为准
 * @retval None
 */
extern void Refresh_QuadrantArea_of_LCD_DDRAM(void)
{
    unsigned char i,j;
    set_window_area(QuadrantAreaStartSeg,QuadrantAreaStartPageCom,QuadrantAreaEndSeg,QuadrantAreaAreaEndPageCom);   //确定LCD的显示区域
    for(i=QuadrantAreaStartPageCom;i<QuadrantAreaAreaEndPageCom;i++)         //页地址范围
    {
        for(j=QuadrantAreaStartSeg;j<QuadrantAreaEndSeg;j++)      //段地址范围
        {
            Writedata(LCDRAM_Buf[j][i]);
        }
    }
}
#endif 

/** 
 * @brief  刷新LCD的中文提示区显示内容
 * @note   是以变量数组LCDRAM_Buf中的内容为准
 * @retval None
 */
extern void Refresh_ChineseHintArea_of_LCD_DDRAM(void)
{
    unsigned char i,j;
    set_window_area(ChineseHintAreaStartSeg,ChineseHintAreaStartPageCom,ChineseHintAreaEndSeg,ChineseHintAreaEndPageCom);   //确定LCD的显示区域
    for(i=ChineseHintAreaStartPageCom;i<ChineseHintAreaEndPageCom;i++)         //页地址范围
    {
        for(j=ChineseHintAreaStartSeg;j<ChineseHintAreaEndSeg;j++)      //段地址范围
        {
            Writedata(LCDRAM_Buf[j][i]);
        }
    }
}

/** 
 * @brief  刷新LCD的数字区显示内容
 * @note   是以变量数组LCDRAM_Buf中的内容为准
 * @retval None
 */
extern void Refresh_NumberArea_of_LCD_DDRAM(void)
{
    unsigned char i,j;
    set_window_area(NumberAreaStartSeg,NumberAreaStartPageCom,NumberAreaEndSeg,NumberAreaEndPageCom);   //确定LCD的显示区域
    for(i=NumberAreaStartPageCom;i<NumberAreaEndPageCom;i++)         //页地址范围
    {
        for(j=NumberAreaStartSeg;j<NumberAreaEndSeg;j++)      //段地址范围
        {
            Writedata(LCDRAM_Buf[j][i]);
        }
    }
}

/** 
 * @brief  刷新LCD的报警区显示内容
 * @note   是以变量数组LCDRAM_Buf中的内容为准
 * @retval None
 */
extern void Refresh_AlarmHintArea_of_LCD_DDRAM(void)
{
    unsigned char i,j;
    set_window_area(AlarmHintAreaStartSeg,AlarmHintAreaStartPageCom,AlarmHintAreaEndSeg,AlarmHintAreaEndPageCom);   //确定LCD的显示区域
    for(i=AlarmHintAreaStartPageCom;i<AlarmHintAreaEndPageCom;i++)         //页地址范围
    {
        for(j=AlarmHintAreaStartSeg;j<AlarmHintAreaEndSeg;j++)      //段地址范围
        {
            Writedata(LCDRAM_Buf[j][i]);
        }
    }
}
/** 
 * @brief  刷新LCD的单位区显示内容
 * @note   是以变量数组LCDRAM_Buf中的内容为准
 * @retval None
 */
extern void Refresh_UnitArea_of_LCD_DDRAM(void)
{
    unsigned char i,j;
    set_window_area(UnitAreaStartSeg,UnitAreaStartPageCom,UnitAreaEndSeg,UnitAreaEndPageCom);   //确定LCD的显示区域
    for(i=UnitAreaStartPageCom;i<UnitAreaEndPageCom;i++)         //页地址范围
    {
        for(j=UnitAreaStartSeg;j<UnitAreaEndSeg;j++)      //段地址范围
        {
            Writedata(LCDRAM_Buf[j][i]);
        }
    }
}
/** 
 * @brief  刷新LCD的状态区显示内容
 * @note   是以变量数组LCDRAM_Buf中的内容为准
 * @retval None
 */
extern void Refresh_StatusHintArea_of_LCD_DDRAM(void)
{
    unsigned char i,j;
    set_window_area(StatusHintAreaStartSeg,StatusHintAreaStartPageCom,StatusHintAreaEndSeg,StatusHintAreaEndPageCom);   //确定LCD的显示区域
    for(i=StatusHintAreaStartPageCom;i<StatusHintAreaEndPageCom;i++)         //页地址范围
    {
        for(j=StatusHintAreaStartSeg;j<StatusHintAreaEndSeg;j++)      //段地址范围
        {
            Writedata(LCDRAM_Buf[j][i]);
        }
    }
}



/** 
 * @brief  写LCDRAM_Buf数组函数
 * @note   
 * @param  x: seg值
 * @param  y: page值
 * @param  Buf: 待写的内容
 * @retval None
 */
extern void Wirte_LCDRAM_Buf(unsigned short x, unsigned short y, unsigned char Buf)
{
    if((x>=seg)||(y>=Page))    //超范围
    {
        return;
    }
    LCDRAM_Buf[x][y] = Buf;

}
/*end-------------------------------------------------------------------------*/

