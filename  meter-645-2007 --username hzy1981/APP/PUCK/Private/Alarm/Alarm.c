#define ALARM_C
#include "Pub_PUCK.h"
#include "Alarm.h"


#if REDEF_FILE_LINE_EN > 0
#line  __LINE__ "A0"
#endif


/***********************************************************************
函数功能：根据位或者字节单位，获取对应的位信息或字节信息
入口：无
出口：无
电能表在检测到异常（时钟电池欠压、有功电能方向改变（双向计量除外）、失压、断相，
以及过压、电压逆相序、失流、电流不平衡、电流逆相序、过流、需量超限、过载等）时
除在循环显示的异常报警代码中进行指示外，报警指示灯也应同时点亮。
***********************************************************************/
// LED，LCD肯定报警，辅助端子排模糊。报警门限1，蜂鸣器不叫 Get_Prepaid_Status() PREPAID_MONEY_LEFT1
void Sound_LED_LCD_Port_Alarm(void)
{
  INT8U i,temp;
  static S_Int8U LastSatus={CHK_BYTE,0x00,CHK_BYTE};
  static INT8U Blink=0;
  
   for(i=0;i<ALARM_EVENT_NUM;i++)
   {
      if(Read_Event_Status(Alarm_Event_Property[i]))  //此事件发生
      {         
        Port_Out_Pub(INTER_ID_ALARM_DIS,300);
        Port_Out_Pub(EXT_ID_AUX_ALARM,AUX_PORT_PULSE_WD);         
        Set_Event_Instant(ID_ALARM_OUT);
        LastSatus.Var=0x05;
         break;      
      }
   }
   if(i>=ALARM_EVENT_NUM)
   {
     Clr_Event_Instant(ID_ALARM_OUT);
     if(LastSatus.Var EQ 0x05)
     {
       Port_Out_Pub(INTER_ID_ALARM_DIS,PORT_END);   //电平方式，自动关闭，脉冲方式，不会调用
       Port_Out_Pub(EXT_ID_AUX_ALARM,PORT_END);  
       LastSatus.Var=0;
     }
   }
   
   temp=Get_Relay_Status();
   if(temp EQ SWITCH_DELAY) //拉闸延时，蜂鸣器叫
      Port_Out_Pub(INTER_ID_ALARM_BEEP,300);

   if(temp EQ SWITCH_ON)
      Port_Out_Pub(INTER_ID_TOGGLE_DIS,PORT_END);  //合闸指示
   
   if(temp EQ SWITCH_OFF)
      Port_Out_Pub(INTER_ID_TOGGLE_DIS,PORT_START);   //跳闸指示
    
    
   if(temp EQ ALLOW_SWITCH_ON)  //远程允许合闸,跳闸指示灯闪烁（亮1s，灭1s）
   {
     if(Blink)
      Port_Out_Pub(INTER_ID_TOGGLE_DIS,PORT_START);  //拉闸指示
     else
      Port_Out_Pub(INTER_ID_TOGGLE_DIS,PORT_END);  //合闸指示
     
     Blink=(Blink EQ 0)?1:0;
   }
      
   if(Get_Prepaid_Status() EQ PREPAID_MONEY_OVERDRAFT) //透支，蜂鸣器叫
   {
     if(Beep_Disable EQ 0)
       Port_Out_Pub(INTER_ID_ALARM_BEEP,300);    
   }
}
/***********************************************************************
void Sound_LED_LCD_Port_Alarm(void)
{
  INT8U  i,j,flag[ALARM_MODE_NUM]={0},flag1,flag2;
   static INT8U num=0;
  
   for(i=0;i<ALARM_EVENT_NUM;i++)
   {
     if(Alarm_Event_Property[i].ModeEn EQ 0 && Read_Event_Status(Alarm_Event_Property[i].EventID))  //输出与模式字无关，强行输出，其他事件报警不必判定
     {
       for(j=0;j<ALARM_MODE_NUM;j++)  //声音报警，LED报警，端子排报警
       {
         Port_Out_Pub(Alarm_Mode_Property[j].PortID,Alarm_Mode_Property[j].AlarmWidth);
         Set_Event_Instant(ID_ALARM_OUT);
         flag[j]=1;        
       }
       flag2=1;
       num=ALARM_MODE_NUM;
       break;
     }
     else                                   //输出与模式字有关
     {
       if(GET_BIT(*(Alarm_Event_Property[i].EventEn),Alarm_Event_Property[i].EventBit))   //是否允许事件报警输出
       {
         if(Read_Event_Status(Alarm_Event_Property[i].EventID))  //确认该事件发生
         {
           for(j=0;j<ALARM_MODE_NUM;j++)  //声音报警，LED报警，端子排报警
           {
             if(GET_BIT(*(Alarm_Mode_Property[j].AlarmEn),Alarm_Mode_Property[j].AlarmBit))  //是否允许报警输出
             {
               Port_Out_Pub(Alarm_Mode_Property[j].PortID,Alarm_Mode_Property[j].AlarmWidth);
               Set_Event_Instant(ID_ALARM_OUT);               
               flag[j]=1;
             }           
           }
           num++;
         }     
       } 
     }  
     
     flag2=0;
     flag1=1;
     for(j=0;j<ALARM_MODE_NUM;j++)  //声音报警，LED报警，端子排报警
     {
       if(flag[j] EQ 0)  //当前报警方式没有输出.继续
         flag1=0;
       else           //当前存在事件发生     
         flag2=1;
     }
     if(flag1)   //所有输出方式都输出了
     {
       num=ALARM_MODE_NUM;
       break;
     }
   }
   
   if(0==flag2)  //无事件发生
   {
      Clr_Event_Instant(ID_ALARM_OUT);
      if(num>=ALARM_MODE_NUM)  //原来有事件，现在结束了
      {
        num=0;
        for(j=0;j<ALARM_MODE_NUM;j++)  //关闭声音报警，LED报警，端子排报警
          Port_Out_Pub(Alarm_Mode_Property[j].PortID,PORT_END);     
      }
   }
}
***********************************************************************/