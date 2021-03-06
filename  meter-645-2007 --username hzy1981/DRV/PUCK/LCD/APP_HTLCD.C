#include "Pub_PUCK.h"


#if REDEF_FILE_LINE_EN > 0
#line  __LINE__ "B7"
#endif


#if LCD_DRV_CHIP==HT1623    
    #if LCD_PROPERTY==LCD_NORMAL
      #define HTLCD_C
      #include "HTLCD_lib.h"
    #endif
    
    #if LCD_PROPERTY==LCD_JS_ZXW
      #define HTLCD_JS_ZXW_C
      #include "HTLCD_lib_JS_ZXW.h"
    #endif
    
    #if LCD_PROPERTY==LCD_JS_JX
      #define HTLCD_JS_JX_C
      #include "HTLCD_lib_JS_ZXW.h"
    #endif
    
    #if LCD_PROPERTY==LCD_SD_ZXW
      #define HTLCD_SD_ZXW_C
      #include "HTLCD_lib_SD_ZXW.h"
    #endif
    
    #if LCD_PROPERTY==LCD_GW_ZXW
      #define HTLCD_GW_ZXW_C
      #include "HTLCD_lib_GW_ZXW.h"
    #endif
 #endif






#if LCD_DRV_CHIP==HT1623 


#define		SYS_DIS_CMD	0x0800	//只取低12位
#define		SYS_EN_CMD      0x0802	//只取低12位
#define		LCD_OFF_CMD	0x0804	//只取低12位
#define		LCD_ON_CMD	0x0806	//只取低12位
#define		RC_32K_CMD	0x0830	//只取低12位
#define		WRITE_ID	0x0280	//低7位与地址相或后，只送最低10位


#define HTLCD_SYS_EN()    HTLCDWriteACommand(SYS_EN_CMD)  //开振荡器
#define HTLCD_SYS_DIS()   HTLCDWriteACommand(SYS_DIS_CMD)  //关振荡器和偏压发生器
#define HTLCD_LCD_ON()    HTLCDWriteACommand(LCD_ON_CMD)  //开LCD显示
#define HTLCD_LCD_OFF()   HTLCDWriteACommand(LCD_OFF_CMD)  //关LCD显示
#define HTLCD_RC_32K()    HTLCDWriteACommand(RC_32K_CMD)  //选择片上RC振荡器

	

//#define SET_BIT  Bit_SET
//#define CLR_BIT  Bit_RESET

#define HTLCD_CS_0        LCD_CS_0
#define HTLCD_CS_1        LCD_CS_1

#define HTLCD_CLK_0       LCD_WR_0
#define HTLCD_CLK_1       LCD_WR_1

#define HTLCD_DATA_0      LCD_DATA_0
#define HTLCD_DATA_1      LCD_DATA_1



/**********************************************************************************
函数功能：获取Sx的seg位置
入口：    ShouID----显示设备ID
出口：    0--------不存在
**********************************************************************************/
INT16U GetPixySegPos_SxNxDx(INT16U DeviceID,INT8U Type)
{
  INT16U i;
  //void *Ptr;
  
  switch(Type)
  {
    case Sx:
      for(i=0;i<MAX_S_NUM;i++)
      {
         if(LCD_S_Const[i].Sign==DeviceID)
          {
            return i;
          } 
      }
    break;
    
    case Dx:
      for(i=0;i<MAX_D_NUM;i++)
      {
         if(LCD_D_Const[i].Sign==DeviceID)
          {
            return i;
          } 
      }
    break;
    
    case Nx:
      for(i=0;i<MAX_N_NUM;i++)
      {
         if(LCD_N_Const[i].Sign==DeviceID)
          {
            return i;
          } 
      }
    break;
    
    default:
    return 0;
  }
  
  /*for(i=0;i<MaxNum;i++)
  {
     if((*)(Ptr[i].Sign==DeviceID)
      {
        return i;
      } 
  }*/
    return 0;
}
/**********************************************************************************
函数功能：点亮或者熄灭某一个显示标志
入口：    Flag----1:显示;0:熄灭
出口：    1---------有此设备；0------找不到设备
**********************************************************************************/
INT8U SetOrClrAndChkSEGCOM(INT16U Seg,INT8U Com,INT8U Type,INT8U ClrOrSet)
{
  if((Seg<=MAX_SEG)&&(Com<=MAX_COL))
  {    
    if(ClrOrSet)
    {
      if(GET_BIT(Show_Lcd_Ram[Seg],Com))  //检查内存是否已经置位或者 seg/com配置表错误
      return 0;
      
      Show_Lcd_Flag=1;
      SET_BIT(Show_Lcd_Ram[Seg],Com);
      return 1;
    }
    else
    {
    	CLR_BIT(Show_Lcd_Ram[Seg],Com);
    	return 1;
    }  
  }
  return 0;
}
/**********************************************************************************
函数功能：点亮或者熄灭某一个显示标志
入口：    Flag----1:显示;0:熄灭
出口：    1---------有此设备；0------找不到设备
**********************************************************************************/
INT8U SetOrClrPixPiyDot(INT16U ShouID,INT8U ClrOrSet,INT8U Type)
{
  INT16U Pos,Seg,Com;
  INT8U Flag,i,temp;
  
  Pos=GetPixySegPos_SxNxDx(ShouID,Type);
  if(!Pos)
    return 0;
  
  switch(Type)
  {
    case Sx:
       Seg=SEG(LCD_S_Const[Pos].x);
       Com=COM(LCD_S_Const[Pos].y);
       Flag=SetOrClrAndChkSEGCOM(Seg,Com,Type,ClrOrSet);
       return Flag;
  
    case Dx:
       Seg=SEG(LCD_D_Const[Pos].x);
       Com=COM(LCD_D_Const[Pos].y);
       Flag=SetOrClrAndChkSEGCOM(Seg,Com,Type,ClrOrSet);
       return Flag;

  case Nx: 
//#ifdef CONFLICT_TAB_CHKEN
//	temp=7;
//#else
	temp=1;
//#endif
      for(i=0;i<temp;i++)
      { 
        Seg=SEG(LCD_N_Const[Pos+i].x);
        Com=COM(LCD_N_Const[Pos+i].y);
      	Flag=SetOrClrAndChkSEGCOM(Seg,Com,Type,ClrOrSet);
      	if(!Flag)
      	return 0;
      }
      return 1;

			
      
	}

  return 0;
}
/**********************************************************************************
函数功能：点亮某一个显示设备
入口：    无
出口：    无
**********************************************************************************/
INT8U SetOnDevice_PUCK(INT16U Device)
{
  INT8U Flag;
  Flag=SetOrClrPixPiyDot(Device,1,Sx);
  return Flag;
}
/**********************************************************************************
函数功能：熄灭某一个显示设备
入口：    无
出口：    无
**********************************************************************************/
INT8U ClrOnDevice_PUCK(INT16U Device)
{
  INT8U Flag;
  Flag=SetOrClrPixPiyDot(Device,0,Sx);
  return Flag;
}

/**********************************************************************************
函数功能：熄灭某一个显示设备
入口：    无
出口：    无
**********************************************************************************/
INT8U SetOnDevice_D_PUCK(INT16U Device)
{
  INT8U Flag;
  Flag=SetOrClrPixPiyDot(Device,1,Dx);
  return Flag;
}
/**********************************************************************************
函数功能：点亮或者熄灭某'8'中的某一个元素:'A'~'G'
入口：    
         offset----具体的设备对应的偏移
         Bit---------------清除还是显示的具体位:0~6('A'~'G')
出口：   1-----------成功;0-------------失败
**********************************************************************************/
INT8U SetOnPixyLED8(INT16U Device,INT8U DisChar)
{
  INT16U Seg,Com,Pos;
  
    //找合理位置
  Pos=GetPixySegPos_SxNxDx(Device,Nx);
  if(!Pos)
    return 0;
    
  if(Pos>=MAX_N_NUM) //防止越界
    return 0;
  
  //检查显示的字段是否合法
  if(DisChar<'A')
    return 0;
  
  if(DisChar>'G')
    return 0;
  
  DisChar=DisChar-'A';
  
  Seg=SEG(LCD_N_Const[Pos+DisChar].x);
  Com=COM(LCD_N_Const[Pos+DisChar].y);
    
  if((Seg<=MAX_SEG)&&(Com<=MAX_COL)) //防止越界
  {
     Show_Lcd_Flag=1;
     SET_BIT(Show_Lcd_Ram[Seg],Com);
  }
  return 1;
}

/**********************************************************************************
函数功能：点亮0~9,A,b,C,d,E,F,...等具有意义的字符，如"9:","2."
入口：    
         DeviceID----具体的'8'设备名称
         DisChar----------点亮0~9,A,b,C,d,E,F,...等具有意义的字符
出口：   1-----------成功;0-------------失败
**********************************************************************************/
INT8U SetOnLED8Device_PUCK(INT16U DeviceID,INT8U DisChar)
{
  INT8U i,CharPos;
  
  //检索LED8显示的内容是否合法：0~9，A,b,C,d,E,F,o,O,...
  for(CharPos=0,i=0;i<MAX_LED_NUM;i++)
  {
    if(LED_Pixy8_Const[i].Type_8==DisChar)
    {
      CharPos=i;
      break;
    }
  }
  //没有找到显示的字符
  if(!CharPos)
    return 0;
  
  //根据 LED_Pixy8_Const 描笔
  for(i=0;i<LED_Pixy8_Const[CharPos].Num;i++)
  {
    SetOnPixyLED8(DeviceID,LED_Pixy8_Const[CharPos].Pxy8[i]);
  }
  return 1;
}

/**********************************************************************************
函数功能：更新显示屏
入口：    Flag----1:表示初始化;0:表示进入主程序调用
出口：    无
**********************************************************************************/
void UpdataLcdShow(void)
{
  if(Show_Lcd_Flag)
  {
    
    //mem_set(Show_Lcd_Ram,0xff,sizeof(Show_Lcd_Ram),Show_Lcd_Ram,sizeof(Show_Lcd_Ram));    
    //LCDWriteMultiData(0,Show_Lcd_Ram,sizeof(Show_Lcd_Ram));
    //mem_set(Show_Lcd_Ram,0x0f,sizeof(Show_Lcd_Ram),Show_Lcd_Ram,sizeof(Show_Lcd_Ram));
    LCDWriteMultiData(0,(INT8U *)Show_Lcd_Ram,sizeof(Show_Lcd_Ram));
    Show_Lcd_Flag=0;
    mem_set((INT8U *)Show_Lcd_Ram,0x00,sizeof(Show_Lcd_Ram),(INT8U *)Show_Lcd_Ram,sizeof(Show_Lcd_Ram));    
  }
}
/**********************************************************************************
函数功能：清除整屏
入口：    无
出口：    无
**********************************************************************************/
void ClearAllScreen(void)
{
  mem_set((INT8U *)Show_Lcd_Ram,0x00,sizeof(Show_Lcd_Ram),(INT8U *)Show_Lcd_Ram,sizeof(Show_Lcd_Ram));    
  Show_Lcd_Flag=1;
  UpdataLcdShow();
}
/**********************************************************************************
函数功能：填满整屏
入口：    无
出口：    无
**********************************************************************************/
void FillAllScreen(void)
{
  mem_set((INT8U *)Show_Lcd_Ram,0xff,sizeof(Show_Lcd_Ram),(INT8U *)Show_Lcd_Ram,sizeof(Show_Lcd_Ram));    
  Show_Lcd_Flag=1;
  UpdataLcdShow();
  //OS_TimeDly_Ms(150);//500ms睡眠
}
/**********************************************************************************
void Show_9_To_0(void)
{  
  INT16U i;
  for(i=1;i<43;i++)
      SetOnDevice_PUCK(i); 
      
  for(i=3;i<13;i++)   
  {
  	SetOnLED8Device_PUCK(N_LED(i),'1'+i-3);
  }
  
  for(i=14;i<22;i++)   
  {
  	SetOnLED8Device_PUCK(N_LED(i),'2'+21-i);
  	if(i==20)
  		SetOnDevice_D_PUCK(D_(20));
  } 
  UpdataLcdShow();  
}
**********************************************************************************/

/**********************************************************************************
函数功能：初始化显示RAM
入口：    无
出口：    无
**********************************************************************************/
void InitLcdAllRam(void)
{
  
 //mem_set(Show_Lcd_Ram,0x00,MAX_SEG+1,Show_Lcd_Ram,MAX_SEG+1);
  //mem_set(Show_Lcd_Ram,0x00,sizeof(Show_Lcd_Ram),Show_Lcd_Ram,sizeof(Show_Lcd_Ram));
  //mem_set(void *pDst, INT8U Value,INT32U SetLen,void *pDst_Start,INT32U Dst_Len)
   mem_set((INT8U *)Show_Lcd_Ram,0x00,sizeof(Show_Lcd_Ram),(INT8U *)Show_Lcd_Ram,sizeof(Show_Lcd_Ram)); 
}

/**********************************************************************************
函数功能：检查配置表是否配置矛盾
入口：    无
出口：    无
**********************************************************************************/
void Chk_Table_Conflict(void)
{
  
#ifdef CONFLICT_TAB_CHKEN
  INT16U i;
  INT8U Flag,Result=1;

  Debug_Print("Check LCD Soft Table Conflict.......................");
  for(i=1;i<MAX_S_NUM;i++)
  {
    Flag=SetOrClrPixPiyDot(LCD_S_Const[i].Sign,1,Sx);
    if(!Flag)
    {
      Result=0;
     Debug_Print("LCD_TABLE Error---->Config Table Error,Sign=S%d\n",LCD_S_Const[i].Sign); 
    }
  }
  
  for(i=1;i<MAX_D_NUM;i++)
  {      
    Flag=SetOrClrPixPiyDot(LCD_D_Const[i].Sign,1,Dx);
    if(!Flag)
    {
      Result=0;
     Debug_Print("LCD_TABLE Error---->Config Table Error,Sign=D%d\n",LCD_D_Const[i].Sign); 
    }
  } 
  
  for(i=1;i<MAX_N_NUM;i+=7)  //7个seg为一个单位
  {      
    Flag=SetOrClrPixPiyDot(LCD_N_Const[i].Sign,1,Nx);
    if(!Flag)
    {
      Result=0;
     Debug_Print("LCD_TABLE Error---->Config Table Error,Sign=N%d\n",LCD_N_Const[i].Sign); 
    }
  }
  if(Result)
    Debug_Print("LCD Soft Table no Conflicet,Succeed!");
  else
    Debug_Print("LCD Soft Table Conflicet,Failed!");
  
  InitLcdAllRam();
#endif
}
/**********************************************************************************
**********************************************************************************/
void Test_Lcd_Proc(void)
{
  INT8U i,j;  
  for(i=0;i<=9;i++)
  {
    for(j=0;j<=i;j++)
    {
      SetOnLED8Device_PUCK(N_LED(12-j),'0'+i-j);
      //SetOnLED8Device_PUCK(N_LED(13+j),'0'+i-j);
    }  
    UpdataLcdShow();
    OS_TimeDly_Ms(80);
  }
  OS_TimeDly_Sec(1);
}

/**********************************************************************************
函数功能：写1个指令到LCD
入口：    指令内容
出口：    无
测试SCLK频率：一个周波5.2us
**********************************************************************************/
void	HTLCDWriteACommand(INT32U command)
{
  INT8U i;
  HTLCD_CS_0;	//CS的数据口置低
  for(i=0;i<=11;i++)		//命令只取最低12位
  {
    HTLCD_CLK_0;	//CLK的数据口
    if((command&0x0800))	//先高位再低位
      HTLCD_DATA_1;
    else
      HTLCD_DATA_0;

    HTLCD_CLK_1;		//CLK的数据口

    command<<=1;
  }
  HTLCD_CS_1;
  HTLCD_DATA_1;
}

/**********************************************************************************
函数功能：使LCD写数据模式
入口：    指令内容
出口：    无
**********************************************************************************/
void	HTLCDInWriteMode(INT8U LCD_RAM_ADDR)
{
  INT32U i,command;
  
  command=WRITE_ID|((INT32U)LCD_RAM_ADDR);
  for(i=0;i<=9;i++)			//只取最低10位
  {
    HTLCD_CLK_0;		//CLK的数据口
    if((command&0x0200))		//先高位再低位
      HTLCD_DATA_1;
    else
      HTLCD_DATA_0;

    HTLCD_CLK_1;		//CLK的数据口

    command<<=1;
  }
  HTLCD_DATA_1;
}

/**********************************************************************************
函数功能：写数据到LCD
入口:
        LCD_RAM_ADDR----------HTLCD LCD RAM 地址(0-95)
        DATA_ADDR-------------显示数据的缓冲区地址
        DATA_LENGTH-----------显示数据的长度
出口：  无
**********************************************************************************/
void LCDWriteMultiData(INT8U LCD_RAM_ADDR,INT8U *DATA_ADDR,INT32U DATA_LENGTH)
{
  INT32U i,j;
  INT8U DATA_BYTE;
  HTLCD_CS_0;
  HTLCDInWriteMode(LCD_RAM_ADDR); //使HTLCD进入写数据模式
  for(i=0;i<DATA_LENGTH;i++)
  {
    DATA_BYTE=*(DATA_ADDR+i);
    for(j=0;j<=7;j++)
    {
      HTLCD_CLK_0;	//WR的数据口
      if((DATA_BYTE&0x01))	//显示数据顺序：D0/D1/D2/D3
        HTLCD_DATA_1;	//DATA的数据口
      else
        HTLCD_DATA_0;	//DATA的数据口
      HTLCD_CLK_1;	
      DATA_BYTE>>=1;
    }
  }
  HTLCD_CS_1;
  HTLCD_DATA_1;
}
/**********************************************************************************
函数功能：工作模式下初始化LCDIO口
入口：    无
出口：    无
**********************************************************************************/
void Init_LCD_Mode(INT32U Mode,INT8U Status)
{  
  switch(Mode)
  {
    case SYS_NORMAL:
        MAIN_POWER_FOR_LCD; 
        //InitHTLCDIOPort();
        HTLCD_SYS_DIS();	//关振荡器和偏压发生器
        HTLCD_LCD_OFF();	//关LCD显示
        HTLCD_RC_32K();	//选择片上RC振荡器
        HTLCD_SYS_EN();	//开振荡器"*/
        HTLCD_LCD_ON();	//开LCD显示  
        FillAllScreen();
    break;
    
    case SYS_RESUME:
        if(Status EQ RESUME_STATUS_PUCK)
          break;
        
        BAK_POWER_FOR_LCD;  
        //InitHTLCDIOPort();
        HTLCD_SYS_DIS();	//关振荡器和偏压发生器
        HTLCD_LCD_OFF();	//关LCD显示
        HTLCD_RC_32K();	//选择片上RC振荡器
        HTLCD_SYS_EN();	//开振荡器
        HTLCD_LCD_ON();	//开LCD显示 
        Turn_Light_Off();  
        ClearAllScreen();
    break;
    
    case SYS_SLEEP:
        //InitHTLCDIOPort(); 
        HTLCD_SYS_DIS();		//关振荡器和偏压发生器
        HTLCD_LCD_OFF();		//关LCD显示
        HTLCD_RC_32K();		//选择片上RC振荡器	
        Turn_Light_Off();
    break;    
  }   
}
/**********************************************************************************
函数功能：定时初始化LCD，避免LCD死机
入口：    无
出口：    无
**********************************************************************************/
void Init_LCD_Loop(void)
{
  static volatile S_Int32U Sec_Timer_Bak={CHK_BYTE,0xFFFFFFFF,CHK_BYTE};
 
  if(Sec_Timer_Pub-Sec_Timer_Bak.Var>=15)
  {
    if(Get_Sys_Status()==SYS_NORMAL)
    {
      MAIN_POWER_FOR_LCD; 
      HTLCD_SYS_DIS();	//关振荡器和偏压发生器
      HTLCD_LCD_OFF();	//关LCD显示
      HTLCD_RC_32K();	//选择片上RC振荡器
      HTLCD_SYS_EN();	//开振荡器"*/
      HTLCD_LCD_ON();	//开LCD显示  
    }
    else if(Get_Sys_Status()==SYS_RESUME)
    {
      BAK_POWER_FOR_LCD; 
      HTLCD_SYS_DIS();	//关振荡器和偏压发生器
      HTLCD_LCD_OFF();	//关LCD显示
      HTLCD_RC_32K();	//选择片上RC振荡器
      HTLCD_SYS_EN();	//开振荡器"*/
      HTLCD_LCD_ON();	//开LCD显示  
    }
    else
    {
      
      
    }
    Sec_Timer_Bak.Var=Sec_Timer_Pub;
  } 
}

 #endif
