#define LOW_POWER_C
#include "Includes.h"

#if REDEF_FILE_LINE_EN > 0
#line  __LINE__ "H16"
#endif


//检查电源状态，返回1
INT8U Check_Power_Status()
{
  TRACE();
  if(JUDGE_POWER_OFF)
  {
    return POWER_DOWN_STATUS;
  }
  else
  {
    return POWER_ON_STATUS;
  }
}

void Soft_Restart()
{
  Debug_Print("----Power_Down!Soft Reset!----");
  Soft_Reset();
}

//掉电中断处理函数
void Power_Down_Proc()
{
  PwrCtrl_ExtDevice_HigSpeed();
  //Set_Sys_Status(SYS_SLEEP);
  //设置系统状态，不要调用Set_Sys_Status()打印调试信息 
  Power_Down_Int_Proc();
  //Save_PD_Data();
  //Soft_Reset();
}

//掉电中断处理函数
void Power_Down_Int_Proc()
{
  Trace_Info.Entry_Flag = 0;//防止重入报错

  Sys_Status.Last_Status = Sys_Status.Status;//先保存上次状态
  Sys_Status.Status = SYS_SLEEP;
  SET_SYS_STATUS_SUM();

  JMP_PD_PROC();
  /*    
    if(CHECK_SYS_STATUS_SUM())//校验和正确
    {
    if(SYS_NORMAL!=Sys_Status.Status)//当前仍然在正常状态
    ASSERT_FAILED();
    //{
    Sys_Status.Last_Status=Sys_Status.Status;//先保存上次状态
    Sys_Status.Status=SYS_SLEEP;
    SET_SYS_STATUS_SUM();
    
    JMP_PD_PROC();
    //}
    //else
    
    //else 当前不是在电源正常情况下，说明目前是唤醒状态！
    }
    else//校验和不正确的情况下，设为睡眠状态并复位
    {
    Sys_Status.Last_Status=Sys_Status.Status;//先保存上次状态
    Sys_Status.Status=SYS_SLEEP;
    SET_SYS_STATUS_SUM();
    Soft_Restart();    
    }  
  */
}

//掉电检测函数
#if 1
void Power_Down_Check()
{
  static S_Int8U PD_Flag={CHK_BYTE,0,CHK_BYTE};
  static volatile S_Int32U Ms_Timer={CHK_BYTE,0,CHK_BYTE};
  //INT32U Temp;
  
  if(CHECK_STRUCT_VAR(PD_Flag) EQ 0)
  ASSERT_FAILED();
  
  if(0x55 EQ PD_Flag.Var)//0x55表示函数重入了 
  return;//ASSERT_FAILED();
  
  PD_Flag.Var=0x55;
  //掉电了而且当前在正常状态才进行掉电检测处理
  if(Check_Power_Status()!=POWER_ON_STATUS)
  {
    if(CHECK_SYS_STATUS_SUM())//校验和正确
    {
      if(SYS_NORMAL EQ Sys_Status.Status)//当前仍然在正常状态
      {
        Sys_Status.Last_Status=Sys_Status.Status;//先保存上次状态
        Sys_Status.Status=SYS_SLEEP;
        SET_SYS_STATUS_SUM();
        JMP_PD_PROC();
        //Save_PD_Data();
        //Soft_Restart();
      }
  //else 当前不是在电源正常情况下，说明目前是唤醒状态！
    }
    else//校验和不正确的情况下，设为睡眠状态并复位
    {
      Sys_Status.Last_Status=Sys_Status.Status;//先保存上次状态
      Sys_Status.Status=SYS_SLEEP;
      SET_SYS_STATUS_SUM();
      Soft_Restart();    
    }
  }
  //计算连续两次监测掉电之间的时间间隔
  /*
  do {Temp=Ms_Timer_Pub;}while(Temp!=Ms_Timer_Pub);
  if(Ms_Timer.Var<Temp)
  {
   if(Temp-Ms_Timer.Var>PD_Chk_Ms.Var)
   PD_Chk_Ms.Var=Temp-Ms_Timer.Var;
  }
  Ms_Timer.Var=Temp;
  */
  PD_Flag.Var=0;
}
#endif

//获取系统初始化标志，如果在主任务初始化过程中返回MAIN_INITING;否则返回0
INT32U Get_Main_Init_Flag()
{
  INT8U Re;
  
  Re = CHECK_STRUCT_VAR(Main_Init_Flag);
  if(Re EQ 0)
  {
    ASSERT_FAILED();
    INIT_STRUCT_VAR(Main_Init_Flag);
    Main_Init_Flag.Var = 0;
  }
  
  return Main_Init_Flag.Var;
}

//设置系统主任务初始化标志
void Set_Main_Init_Flag()
{
  Main_Init_Flag.Var = MAIN_INITING;
}

//清除主任务初始化标志
void Clr_Main_Init_Flag()
{
  Main_Init_Flag.Var = 0; 
}

//写掉电数据
void Save_PD_Data()
{
  INT32U Temp_Timer;

  TRACE();
  //OS_Debug_Print("\r\n----------Save_PD_Data Start----------");
  Ms_Timer_Pub = 0;
  if(Get_Main_Init_Flag() EQ MAIN_INITING)
  {
    Debug_Print("Main Initing....save no PD data"); 
    return;
  }
  
  //OS_CLR_BIT(OS_Sem.Sem,PUB_RW_ROM_ID);//强制释放读写存储器的信号量
  //SET_STRUCT_SUM(OS_Sem);
  //OS_Event[PUB_RW_ROM_ID].Value = 1;
  OS_Mutex_Init(PUB_RW_ROM_ID, 1);
  
  Save_Cur_Energy_PD_Data();//电量数据
  Save_Cur_Demand_PD_Data();//需量数据
  Save_Cur_VolStat_PD_Data();//电压统计数据
  Save_LoadData_PD_Data();
  Save_Event_PD_Data();//保存事件数据
  //Save_PD_Time();//保存掉电时间
  SET_PD_RST_FLAG; //设置掉电重启标志
  
  STOP_1MS;  
  Temp_Timer=Ms_Timer_Pub;
  START_1MS;
  
  Write_Storage_Data(_SDI_SAVE_PD_DATA_TIME,(void *)&Temp_Timer,4);
  Init_DebugChanel_PUCK(0);//初始化调试串口，不需判定工厂状态
  _OS_Debug_Print("\r\n--------Save_PD_Data End,Time:%ld ms----------",Ms_Timer_Pub);
}

//对于某个任务来说具体的低功耗流程，这样处理:
//通过调用Check_Sys_Status()调用获得当前的状态 
//SYS_NORMAL表示正常状态,得到此状态,按正常流程走
//SYS_RESUME表示唤醒状态,得到此状态,按唤醒流程走,通过调用Get_Resume_Source()获得唤醒源
//  根据不同的唤醒源执行不同的操作，执行完后调用Task_Sleep()函数睡眠
//具体的唤醒源，可以调用Get_Resume_Source()获得
//Get_Resume_Source()的返回有:
//具体包括:
//IRAD_RESUME     0x01//红外唤醒
//KEY_RESUME  0x02//按键唤醒
//ALL_LOSS_VOL_RESUME 0x04//全失压唤醒
//BATON_RESUME    0x10//电池上电

//注意:
//上电后，读取Check_Sys_Status(),系统可能处于三种状态:应该根据这三种状态进行不同初始化
//a SYS_SLEEP状态，表示发生了掉电，从正常状态复位而来。各任务应该完成善后工作然后Task_Sleep()睡眠
//
//b SYS_NORMAL状态，表示外部电源正常，按正常流程走。如果发生掉电事件则设置系统状态为SYS_SLEEP状态
//       并马上进行软件复位
//      
//c SYS_RESUME状态，唤醒状态，表示没有外部电源，用电池上电的状态,此时读取唤醒源为BATON_RESUME
//
//
//
//
//
//初始化任务状态为非睡眠状态
void Init_Task_Status()
{
  TRACE();

  mem_set((void *) &Task_Status, 0, sizeof(Task_Status), (void *) &Task_Status, sizeof(Task_Status));
  INIT_STRUCT_VAR(Task_Status);
  SET_STRUCT_SUM(Task_Status);
}

//初始化唤醒源
void Init_Resume_Source()
{
  TRACE();

  Resume_Src.Src_Flag = 0;
  //mem_set((void *)&Resume_Src,0,sizeof(Resume_Src),(void *)&Resume_Src,sizeof(Resume_Src));
  INIT_STRUCT_VAR(Resume_Src);
  //SET_STRUCT_SUM(Resume_Src);  
}

//检查系统状态的合法性,此函数要在Sys_Status_Proc函数调用之后才能够调用
void Check_Sys_Status_Avail()
{
  //TRACE();
  Check_Sys_Status();
}

INT8U Get_Power_Status()
{
  INT8U i;

  TRACE();

  for(i = 0; i < 20; i++)
  {
    if(Check_Power_Status() != POWER_ON_STATUS)//电压正常情况下
      break;
    OS_TimeDly_Ms(20);
  }
  //连续400ms判定电压是稳定的
  if(i EQ 20)
  {
    return POWER_ON_STATUS;
  }
  else
  {
    return POWER_DOWN_STATUS;
  }
}

//获取系统当前的状态,初次上电返回SYS_NULL
INT32U Get_Sys_Status()
{
  INT8U Re;

  TRACE();

  Re = 1;
  Re &= CHECK_STRUCT_VAR(Sys_Status);
  Re &= CHECK_SYS_STATUS_SUM();
  if(1 EQ Re && (SYS_NORMAL EQ Sys_Status.Status || \
   SYS_SLEEP EQ Sys_Status.Status || \
   SYS_RESUME EQ Sys_Status.Status))
  {
    return Sys_Status.Status;
  }
  else if(Check_Power_Status() EQ POWER_ON_STATUS)//在校验和或者系统状态不正确的情况下，需要直接查询掉电io口电平
  {
    Sys_Status.Status = SYS_NORMAL;
    SET_SYS_STATUS_SUM();
    return SYS_NORMAL;
  }
  else//使用电池初次上电,设置为唤醒状态，认为是电池上电
  {
    Sys_Status.Status = SYS_RESUME;
    SET_SYS_STATUS_SUM();
    return SYS_RESUME;
  }
}

//获取系统的前次状态
INT32U Get_Sys_Last_Status()
{
  INT8U Re;

  TRACE();

  Re = CHECK_SYS_STATUS_SUM();
  if(1 EQ Re && (SYS_NORMAL EQ Sys_Status.Last_Status || \
   SYS_SLEEP EQ Sys_Status.Last_Status || \
   SYS_RESUME EQ Sys_Status.Last_Status))
  {
    return Sys_Status.Last_Status;
  }
  else
  {
    Sys_Status.Last_Status = SYS_NULL;
    return SYS_NULL;
  }
}

//该函数等同于Get_Sys_Status
INT32U Check_Sys_Status()
{
  return Get_Sys_Status();
}

//设置唤醒源
void Set_Resume_Source(INT8U Src)
{
  Resume_Src.Src_Flag |= Src;
}

//判断某个源是否是中断源,Src为中断源类型
//将某个中断源类型取反作为参数传入，可判断该源以外的所有源是否有一个是当前中断源
INT8U Check_Resume_Source(INT8U Src)
{
  if((Resume_Src.Src_Flag & Src) != 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
//设置系统当前的状态
void Set_Sys_Status(INT32U Status)
{
  TRACE();

  if(SYS_SLEEP EQ Status)
  {
    Debug_Print("..........System Sleep..........");
  }
  else if(SYS_RESUME EQ Status)
  {
    Debug_Print("..........System Resume..........");
  }
  else if(SYS_NORMAL EQ Status)
  {
    Debug_Print("..........System Normal..........");
  }

  Sys_Status.Status = Status;
  SET_SYS_STATUS_SUM();

  if(SYS_RESUME EQ Status || SYS_NORMAL EQ Status)
  {
    Init_CPU_PUCK(Status);
    Init_ExtDevice_PUCK(Status);
  }
  else if(SYS_SLEEP EQ Status)
  {
    Init_ExtDevice_PUCK(Status);//关闭外部设备
    Init_CPU_PUCK(Status);//关闭内部设备，并CPU进入睡眠状态,清唤醒源操作也在该函数内执行
  }
}

//等待所有任务都睡眠,Ms毫秒
void Wait_Tasks_Sleep(INT32U Ms)
{
  INT32U Counts;

  TRACE();

  Counts = 0;
  while(1)//此时各任务已经唤醒,等待各子任务完成唤醒处理流程后继续睡眠
  {
    if(Check_Tasks_Sleep_Done() || Counts >= Ms)//任务完成睡眠或者睡眠处理过程超时
      break;

    Task_Monitor_Proc();//检测各任务执行

    Clear_Task_Dog();   //清任务看门狗,低功耗模式下不要清外部看门狗
    Clear_CPU_Dog();  //清CPU内部看门狗

    OS_TimeDly_Ms(100);
    Counts += 100;
  }

  if(Counts >= Ms)
  {
    Debug_Print("Wait_Tasks_Sleep TimeOut");
  }
}

void Init_Low_Power_Ram()
{
  TRACE();

  PD_INT_EN_Flag = 1;//初始化时掉电中断时打开的
  Resume_Src.Src_Flag = 0;
  INIT_STRUCT_VAR(Resume_Src);
}

/*
//三、各子任务的掉电与唤醒处理流程
if(Get_Sys_Status() EQ SYS_SLEEP)//发生了掉电，完成睡眠前的善后处理
{
  完成善后处理;
  Task_Sleep();//该任务睡眠,然后监控任务让CPU睡眠,如果有唤醒，则该任务从该函数返回继续执行 
  ;此处该任务被唤醒，可在此处直接进行唤醒相关操作,也可按下面的流程判断是否被唤醒
}

if(Get_Sys_Status() EQ SYS_RESUME)//被唤醒了，完成唤醒相关的工作
{
  完成唤醒相关操作;
  Task_Sleep();//该任务睡眠,等待监控任务唤醒,如果有唤醒，则该任务从该函数返回继续执行
}
*/
//四、初始化过程不用做特殊处理，参数数据等无论停电上电状态都应该读出来。
void Init_Low_Power_Proc()
{
  INT32U Status;

  TRACE();

  Init_Task_Status();//设置所有任务为非睡眠状态，睡眠状态只用于低功耗下Task_Sleep
  Init_Resume_Source();//初始化唤醒源 

  if(Get_Power_Status() EQ POWER_ON_STATUS)//电压正常情况下,设置系统状态为正常
  {
    INIT_STRUCT_VAR(Sys_Status);
    Sys_Status.Status = SYS_NORMAL;
    SET_SYS_STATUS_SUM();
  }

  Status = Check_Sys_Status();
  if(SYS_NORMAL EQ Status && Get_Power_Status() EQ POWER_ON_STATUS)//当前电源正常
  {
    //设置当前状态为正常状态,同时上次的状态得以保存在Last_Status中  
    //可以通过调用Get_Sys_Last_Status()获得
    Set_Sys_Status(SYS_NORMAL);
    Init_Clock(SYS_NORMAL);
  }
  else
  {
    Set_Sys_Status(SYS_RESUME);//设置系统状态为睡眠态
    Init_Clock(SYS_RESUME);//电池唤醒的情况下需要初始化下时钟

    //HUCK处理,设置系统状态为睡眠状态,中断唤醒程序可通过查询Get_Sys_Status知道当前是从休眠或者正常上电情况下唤醒的
    Set_Sys_Status(SYS_SLEEP);//设置系统状态为睡眠态
    Set_Sys_Status(SYS_RESUME);//设置系统状态为睡眠态
  }
}

//任务睡眠
void Task_Sleep()
{
  INT8U Re, Cur_Task_ID;

  TRACE();

  Re = CHECK_STRUCT_SUM(Task_Status);
  if(1 != Re)
  {
    ASSERT_FAILED();
    Check_Data_Avail();
  }

  //Check_Task_Para();//检查任务参数
  Cur_Task_ID = OS_Get_Cur_Task_ID();  
  SET_BIT(Task_Status.Sleep_Flag, Cur_Task_ID);
  SET_STRUCT_SUM(Task_Status);
  OS_Waitfor(GET_BIT(Task_Status.Sleep_Flag, Cur_Task_ID) EQ 0);//等待唤醒
}

//唤醒所有睡眠的任务
void Resume_Tasks()
{
  INT8U Re;

  TRACE();

  Re = CHECK_STRUCT_SUM(Task_Status);
  if(1 != Re)//ASSERT(1 EQ Re))
  {
    ASSERT_FAILED();
    Check_Data_Avail();
  }

  mem_set((void *) &Task_Status, 0, sizeof(Task_Status), (void *) &Task_Status, sizeof(Task_Status));
  SET_STRUCT_SUM(Task_Status);
}

//判断任务号为Task_ID的任务的状态是否睡眠 
//返回1表示已经睡眠,0表示还没有睡眠
INT8U Check_Task_Sleep(INT8U Task_ID)
{
  INT8U Re;

  TRACE();

  Re = CHECK_STRUCT_SUM(Task_Status);
  if(1 != Re)//ASSERT(1 EQ Re))
  {
    ASSERT_FAILED();
    Check_Data_Avail();
  }  

  if(GET_BIT(Task_Status.Sleep_Flag, Task_ID) EQ 0)//第i号任务没有睡眠
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

//检查所有任务是否已经都睡眠,1表示除了本身任务以外都已经睡眠,0表示除了自己还有任务没有睡眠
//该函数提供给监控任务调用
INT8U Check_Tasks_Sleep_Done()
{
  INT8U Re;
  INT8U i, Cur_Task_ID, Cur_Task_Num;

  TRACE();

  Re = CHECK_STRUCT_SUM(Task_Status);
  if(1 != Re)//ASSERT(1 EQ Re))
  {
    ASSERT_FAILED();
    Check_Data_Avail();
  }

  Cur_Task_ID = OS_Get_Cur_Task_ID();
  Cur_Task_Num = OS_Get_Cur_Task_Num();  
  for(i = 0; i < Cur_Task_Num; i++)//除当前任务以外的所有任务都已经睡眠?
  {
    if(i != Cur_Task_ID)//当前任务是监控任务，不判断
    {
      if(GET_BIT(Task_Status.Sleep_Flag, i) EQ 0)//第i号任务没有睡眠
      {
        return 0;
      }
    }
  }
  return 1;//所有任务都已经睡眠了
}

//二、主任务的掉电与唤醒处理流程
void Low_Power_Proc()
{
  TRACE();

  if(Check_Power_Status() EQ POWER_DOWN_STATUS)//该函数由PUCK调用suck底层函数完成，查询方式、应为抽象层函数
  {
    while(1)//此While(1)不能退出，只能CPU复位才能退出该循环
    {
      //当前的中断源只有全失压则继续睡眠,否则应该唤醒各任务执行
      Set_Sys_Status(SYS_RESUME);//HUCK处理,设置系统状态为唤醒状态,各子任务可通过Get_Sys_Status()函数得到当前状态   
      Resume_Tasks();//HUCK完成,唤醒各个子任务
      Wait_Tasks_Sleep(300000);//最多等待300s
      Set_Sys_Status(SYS_SLEEP);//设置系统状态为睡眠态
    }
  }
}

//检查低功耗相关数据是否合法
void Check_Low_Power_Data_Avail()
{
  INT8U Re;

  TRACE();

  Re = 1;
  Re &= CHECK_STRUCT_VAR(Sys_Status);
  Re &= CHECK_SYS_STATUS_SUM();
  //ASSERT(1 EQ Re);

  //Re = 1;
  Re &= CHECK_STRUCT_VAR(Task_Status);
  Re &= CHECK_STRUCT_SUM(Task_Status);
  //ASSERT(1 EQ Re);

  //Re = 1;
  Re &= CHECK_STRUCT_VAR(Resume_Src);
  ASSERT(1 EQ Re);
}

//初始化系统状态变量的头和尾
void Init_Sys_Status()
{
  TRACE();

  INIT_STRUCT_VAR(Sys_Status);
}
#undef LOW_POWER_C