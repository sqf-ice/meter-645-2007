#define    AllLossMeasu_C
#include "Pub_PUCK.h"

#if REDEF_FILE_LINE_EN > 0
#line  __LINE__ "A5"
#endif



#if ALL_LOSS_TYPE  EQ ALL_LOSS_SOFT    
    #define ALL_LOSS_FEED_DOG() Clr_Ext_Inter_Dog()   //低成本下，外部狗也要喂
#else    
    #define ALL_LOSS_FEED_DOG() Clear_CPU_Dog()
#endif

    
    
extern const INT32U I_RATE_CONST[];

/***********************************************************************
函数功能：将全失压数据清零，调用条件：fx1晶振，在掉电存数中调用，为开始计全失压做准备
入口：无
出口：无
***********************************************************************/
void Clr_All_Loss_Data(void)
{
  INT8U i;
  mem_set((void *)(&All_Loss_Var),0,sizeof(All_Loss_Var),\
                   (void *)(&All_Loss_Var),sizeof(All_Loss_Var));
  All_Loss_Var.Status.First=1;
  SET_VAR_CS_PUCK(All_Loss_Var.Status); 
  for(i=0;i<ALL_LOSS_NUM;i++)
    SET_VAR_CS_PUCK(All_Loss_Var.RecordTime[i]);
}

/***********************************************************************
函数功能：根据CS成立条件，清零全失压sram，调用条件：fx1晶振，程序复位后
入口：无
出口：无
***********************************************************************/
void Init_All_Loss_Sram(void)
{
  INT8U Flag,i;
  
  Flag=CHK_VAR_CS_PUCK(All_Loss_Var.Status);
  for(i=0;i<ALL_LOSS_NUM;i++)
    Flag&=CHK_VAR_CS_PUCK(All_Loss_Var.RecordTime[i]);
  
  if(!Flag)
  {
    ASSERT_FAILED();  
    mem_set((void *)(&All_Loss_Var),0,sizeof(All_Loss_Var),(void *)(&All_Loss_Var),sizeof(All_Loss_Var));
    All_Loss_Var.Status.First=1;
    SET_VAR_CS_PUCK(All_Loss_Var.Status); 
    for(i=0;i<ALL_LOSS_NUM;i++)
    SET_VAR_CS_PUCK(All_Loss_Var.RecordTime[i]); 
  }
}

/***********************************************************************
函数功能：存储全失压数据，调用条件：fx1晶振，事件任务已经开启，并且是主电源来电
入口：无
出口：无
***********************************************************************/
void Save_All_Loss_Data(void)
{
  INT8U i,temp,num;
  
#if ALL_LOSS_TYPE EQ ALL_LOSS_HARD_SINGLE
  INT32U temp32;
#endif

  
  
  temp=CHK_VAR_CS_PUCK(All_Loss_Var.Status);
  if(0==temp)
    return;   



  if((All_Loss_Var.Status.Nums==0)||(All_Loss_Var.Status.Mins==0))   //没有事件发生
  {
    Clr_All_Loss_Data();   //存完后清0
    return ;
  }
   
#if ALL_LOSS_TYPE EQ ALL_LOSS_HARD_SINGLE    
    Get_Time_Diff(PD_Time,&temp32);    
    All_Loss_Var.Status.Nums=1;
    All_Loss_Var.Status.Mins=temp32;
    if(All_Loss_Var.Status.Mins EQ 0)
      All_Loss_Var.Status.Mins=1;      
#endif
    
  if(All_Loss_Var.Status.start && All_Loss_Var.Status.Index<ALL_LOSS_NUM)  //有发生没有结束,且不越界
  {
    if((CHeck_Same_Byte((INT8U*)All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime,0x00,sizeof(All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime))))
    {
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[0]=Cur_Time1.Sec;
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[1]=Cur_Time1.Min;
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[2]=Cur_Time1.Hour;
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[3]=Cur_Time1.Date;
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[4]=Cur_Time1.Month;
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[5]=Cur_Time1.Year;  //取系统时钟
      SET_VAR_CS_PUCK(All_Loss_Var.RecordTime[All_Loss_Var.Status.Index]);
    }
  }
  num=(All_Loss_Var.Status.Nums<=ALL_LOSS_NUM)?All_Loss_Var.Status.Nums:ALL_LOSS_NUM;
  if(All_Loss_Var.Status.Index<ALL_LOSS_NUM)   //index保证不越界
  {
     for(i=0;i<num;i++)  
     {
       if(num<=ALL_LOSS_NUM)  //没有翻转：按顺序依次写
         temp=i;   
       else                   //翻转：按老-->新依次写  
        temp=((All_Loss_Var.Status.Index+1)%ALL_LOSS_NUM+i)%ALL_LOSS_NUM;       
      All_Loss_Vol_Data_Proc((INT8U *)All_Loss_Var.RecordTime[temp].StartTime,(INT8U *)All_Loss_Var.RecordTime[temp].EndTime,All_Loss_Var.Curr[temp]);
     }
  }
  
  All_Loss_Vol_Counts_Time_Proc(All_Loss_Var.Status.Nums,All_Loss_Var.Status.Mins); 
  Clr_All_Loss_Data();   //存完后清0  
}

/***********************************************************************
函数功能：计算全失压下的电流值
入口：无
出口：1---------电流值满足全失压的电流值;0----------电流不满足全失压的电流值
***********************************************************************/
INT8U Get_AllLoss_Curr(void)
{

  INT16U i;
  INT32U RdData;
  FP32S  ResultData,Temp,JudgeIn;
  INT8U Flag,OccurFlag;
  
   BAT_ON_7022;     //打开后备电池
   
   PM13_bit.no0=0;   //7022_CS
   PM2_bit.no0=0;    //计量RST---------7022_RST   
   PM2_bit.no2=0;    //计量SDO---------7022_SDO     
   PM2_bit.no4=0;    //计量SCK---------7022_SCK
   
   PM2_bit.no1=1;    //计量SIG---------7022_SIG   
   PM2_bit.no3=1;    //计量SDI ---------7022_SDI  

  ALL_LOSS_FEED_DOG();   
   
  for(i=0;i<10;i++)
    WAITFOR_DRV_CYCLE_TIMEOUT(CYCLE_NUM_IN_1MS);  
 
  ALL_LOSS_FEED_DOG();
  MEASU_RST_0;
  for(i=0;i<10;i++)
    WAITFOR_DRV_CYCLE_TIMEOUT(CYCLE_NUM_IN_1MS);
  
  MEASU_RST_1;
  
   //延时100ms
  for(i=0;i<100;i++)
  {
     WAITFOR_DRV_CYCLE_TIMEOUT(CYCLE_NUM_IN_1MS);   
     ALL_LOSS_FEED_DOG();
  }
   

   EnMeasuCal();  
   //初始化的时候，就需要获取电流规格，电流增益参数
   for(i=0;i<3;i++)
   {
      Flag=Measu_WrAndCompData(REG_W_IGAIN_A+i,Curr_Rate.Rate[i]);
      ALL_LOSS_FEED_DOG();
      if(!Flag)
        break;        
   }   
   DisMeasuCal();
   
   
    //延时100ms
   for(i=0;i<100;i++)
   {
     WAITFOR_DRV_CYCLE_TIMEOUT(CYCLE_NUM_IN_1MS);
      ALL_LOSS_FEED_DOG();
   }
   
   ResultData=0;
   OccurFlag=0;
   JudgeIn=(FP32S)Get_In()/20.0;   //5%In
   for(i=0;i<3;i++)
   {
      Flag=Measu_RdAndCompData(REG_R_A_I+i,(INT8U *)(&RdData));
      ALL_LOSS_FEED_DOG();
      if(!Flag || RdData>=0x00800000)
      {
        break ;
      }
      Temp=((FP32S)RdData*(FP32S)UNIT_A/8192)/(FP32S)I_RATE_CONST[Get_SysCurr_Mode()];
      ResultData+=Temp;
      *(&(Measu_Sign_InstantData_PUCK.Curr.A)+i)=(INT32S)Temp;  //更新公有电流数据，用于显示
      if(Temp/UNIT_A >=JudgeIn)
        OccurFlag=1;
   }
   ALL_LOSS_FEED_DOG();
   if(OccurFlag)
   {
     if(i>=3)
        All_Loss_Var.Curr[All_Loss_Var.Status.Index]=(INT32U)(ResultData/3);   //算出平均电流
     else       
        All_Loss_Var.Curr[All_Loss_Var.Status.Index]=(INT32U)(ResultData/(i+1));
   }
   else  //不是全失压
   {
      All_Loss_Var.Status.Nums=0;    
      All_Loss_Var.Status.Mins=0;     
   }
   
   ALL_LOSS_FEED_DOG();
   
   P13_bit.no0=0;   //7022_CS
   P2_bit.no0=0;    //计量RST---------7022_RST   
   P2_bit.no2=0;    //计量SDO---------7022_SDO     
   P2_bit.no4=0;    //计量SCK---------7022_SCK
   
   PM2_bit.no1=0;    //计量SIG---------7022_SIG   
   PM2_bit.no3=0;    //计量SDI ---------7022_SDI
   P2_bit.no1=0;    //计量SIG---------7022_SIG   
   P2_bit.no3=0;    //计量SDI ---------7022_SDI   
   
   BAT_OFF_7022;   //关闭后备电池
   
   return OccurFlag;  
}
/***********************************************************************
函数功能：计算全失压数据，调用条件：fxt晶振，RTC闹铃。在RTC中断中调用！
入口：无
出口：无
***********************************************************************/
void Hard_All_Loss_Proc(void)
{ 
#if ALL_LOSS_TYPE EQ ALL_LOSS_HARD_SINGLE
  All_Loss_Var.Status.Exist=1;
  All_Loss_Var.Status.start=0;
  All_Loss_Var.Status.First=1;
  
#endif

#if ALL_LOSS_TYPE EQ ALL_LOSS_HARD_MULTI
  if(All_Loss_Var.Status.Occur)
  {
    All_Loss_Var.Status.Occur=0;
    All_Loss_Var.Status.Exist=1;
  }
  else
    All_Loss_Var.Status.Exist=0;
  
  if(All_Loss_Var.Status.Exist==0 && All_Loss_Var.Status.start==0) //无失压发生，以最快的速度退出
  {
    SET_VAR_CS_PUCK(All_Loss_Var.Status); 
    return  ;
  }
#endif
  if(All_Loss_Var.Status.Exist && All_Loss_Var.Status.start==0)  //全失压发生
  {   
   
#if ALL_LOSS_TYPE EQ ALL_LOSS_HARD_SINGLE
  All_Loss_Var.Status.Nums=0;  
  All_Loss_Var.Status.Mins=1;
#endif 

   
    All_Loss_Var.Status.start=1;
    if(All_Loss_Var.Status.First)
    {
      All_Loss_Var.Status.First=0;
      All_Loss_Var.Status.Index=0;
    }
    else
      All_Loss_Var.Status.Index=(All_Loss_Var.Status.Index+1)%ALL_LOSS_NUM;
    
    /*
    if(All_Loss_Var.Status.Index>ALL_LOSS_NUM)  //防止越界
      All_Loss_Var.Status.Index=0;
    */
    All_Loss_Var.Status.Nums++;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[0]=SEC;      //CPU_RTC_Time.RTC.Min;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[1]=MIN;      //CPU_RTC_Time.RTC.Min;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[2]=HOUR;      //CPU_RTC_Time.RTC.Hour;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[3]=DAY;      //CPU_RTC_Time.RTC.Date;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[4]=MONTH;      //CPU_RTC_Time.RTC.Month;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[5]=YEAR;      //CPU_RTC_Time.RTC.Year;
    
    Get_AllLoss_Curr();
    
    memset((INT8U*)All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime,0x00,\
          sizeof(All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime));     //死写，不用mem_set
    
    SET_VAR_CS_PUCK(All_Loss_Var.Status); 
    SET_VAR_CS_PUCK(All_Loss_Var.RecordTime[All_Loss_Var.Status.Index]); 
    return ;
  }
  
  if(All_Loss_Var.Status.Exist && All_Loss_Var.Status.start)    //全失压延续
  {
    All_Loss_Var.Status.Mins++;
    SET_VAR_CS_PUCK(All_Loss_Var.Status);
    return ;
  }
  
  if(All_Loss_Var.Status.Exist==0 && All_Loss_Var.Status.start) //全失压结束
  {
    All_Loss_Var.Status.Mins++;
    All_Loss_Var.Status.start=0;
    
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[0]=SEC;      //CPU_RTC_Time.RTC.Min;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[1]=MIN;      //CPU_RTC_Time.RTC.Min;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[2]=HOUR;     // CPU_RTC_Time.RTC.Hour;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[3]=DAY;      //CPU_RTC_Time.RTC.Date;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[4]=MONTH;    //  CPU_RTC_Time.RTC.Month;
    All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime[5]=YEAR;     // CPU_RTC_Time.RTC.Year;
    
    Measu_Sign_InstantData_PUCK.Curr.A=0;
    Measu_Sign_InstantData_PUCK.Curr.B=0;
    Measu_Sign_InstantData_PUCK.Curr.C=0;
    
    SET_VAR_CS_PUCK(All_Loss_Var.Status); 
    SET_VAR_CS_PUCK(All_Loss_Var.RecordTime[All_Loss_Var.Status.Index]); 
    return ;    
  }
  
  SET_VAR_CS_PUCK(All_Loss_Var.Status); 
  SET_VAR_CS_PUCK(All_Loss_Var.RecordTime[All_Loss_Var.Status.Index]);
}
/***********************************************************************
函数功能：计算全失压数据，调用条件：fxt晶振，RTC闹铃。在RTC中断中调用！
入口：无
出口：无
***********************************************************************/
void Soft_All_Loss_Proc(void)
{
#if ALL_LOSS_TYPE EQ ALL_LOSS_SOFT
  
  
  //if(LOWCOST_BAT_LOW EQ 0) //没有低功耗电池了
  //  return  ;
  
  
  if(CHK_PD_RST_FLAG EQ 0) //当前不是掉电后的正常复位，不去判断全失压:
  {
    CLR_PD_RST_FLAG;
    return ;
  }
    
  Get_Soft_Curr_Rate();//获取增益参数  
   //全失压发生
   if(Get_AllLoss_Curr())
   {
      All_Loss_Var.Status.Index=0;
      All_Loss_Var.Status.start=1;   //有发生没有结束
      All_Loss_Var.Status.Nums=1;
      All_Loss_Var.Status.Mins=1;
      
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[0]=Cur_Time1.Sec;      //CPU_RTC_Time.RTC.Min;
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[1]=Cur_Time1.Min;      //CPU_RTC_Time.RTC.Min;
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[2]=Cur_Time1.Hour;      //CPU_RTC_Time.RTC.Hour;
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[3]=Cur_Time1.Date;      //CPU_RTC_Time.RTC.Date;
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[4]=Cur_Time1.Month;      //CPU_RTC_Time.RTC.Month;
      All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].StartTime[5]=Cur_Time1.Year;      //CPU_RTC_Time.RTC.Year;    
      
      memset((INT8U*)All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime,0x00,\
            sizeof(All_Loss_Var.RecordTime[All_Loss_Var.Status.Index].EndTime));     //死写，不用mem_set    
      SET_VAR_CS_PUCK(All_Loss_Var.Status); 
      SET_VAR_CS_PUCK(All_Loss_Var.RecordTime[All_Loss_Var.Status.Index]);  
   }
   else  //不是全失压
   {
      All_Loss_Var.Status.Nums=0;    
      All_Loss_Var.Status.Mins=0;
   }  
   ALL_LOSS_FEED_DOG();
   CLR_PD_RST_FLAG;   //HUCK需求
   _Debug_Print("All Loss Even Curr=%ld",All_Loss_Var.Curr[All_Loss_Var.Status.Index]);
#endif   
}
/***********************************************************************
函数功能：获取电流增益参数
入口：无
出口：无
***********************************************************************/
void Get_Hard_Curr_Rate(void)
{
#if ALL_LOSS_TYPE !=ALL_LOSS_SOFT 
  INT8U i,Rdflag,temp[3];
  
  for(i=0;i<3;i++)
  {
    INIT_STRUCT_VAR(Curr_Rate);
    Rdflag=Read_Storage_Data_PUCK(DI_I_GAIN_A+i,temp,3);
    if(Rdflag)
    {
      Curr_Rate.Rate[i]=(INT32U)(temp[2]*65536L+temp[1]*256L+temp[0]);      
    }    
  }
#endif
}

/***********************************************************************
函数功能：获取电流增益参数
入口：无
出口：无
***********************************************************************/
void Get_Soft_Curr_Rate(void)
{
#if ALL_LOSS_TYPE  EQ ALL_LOSS_SOFT 
  INT8U i,Rdflag,temp[3];
  
  
  BAK_POWER_FOR_MEM;  //打开内卡电源，获取电流增益
  
  for(i=0;i<3;i++)
  {
    INIT_STRUCT_VAR(Curr_Rate);
    Rdflag=Read_Storage_Data_PUCK(DI_I_GAIN_A+i,temp,3);
    ALL_LOSS_FEED_DOG();
    if(Rdflag)
    {
      Curr_Rate.Rate[i]=(INT32U)(temp[2]*65536L+temp[1]*256L+temp[0]);      
    }    
  }
  MAIN_POWER_FOR_MEM;  //关闭内卡电源
#endif
}



