/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   RTC驱动
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F429 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f4xx.h"
//#include "./Bsp/RTC/bsp_rtc.h"
//#include "./Bsp/usart/bsp_debug_usart.h"
//#include "./lcd/bsp_lcd.h"
#include "bsp_rtc.h"
#include "bsp_debug_usart.h"
 
/**
  * @brief  设置时间和日期
  * @param  无
  * @retval 无
  */
void RTC_TimeAndDate_Set(void)
{
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;
	
	// 初始化时间
	RTC_TimeStructure.RTC_H12 = RTC_H12_AMorPM;
	RTC_TimeStructure.RTC_Hours = HOURS;        
	RTC_TimeStructure.RTC_Minutes = MINUTES;      
	RTC_TimeStructure.RTC_Seconds = SECONDS;      
	RTC_SetTime(RTC_Format_BINorBCD, &RTC_TimeStructure);
	RTC_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);
	
  // 初始化日期	
	RTC_DateStructure.RTC_WeekDay = WEEKDAY;       
	RTC_DateStructure.RTC_Date = DATE;         
	RTC_DateStructure.RTC_Month = MONTH;         
	RTC_DateStructure.RTC_Year = YEAR;        
	RTC_SetDate(RTC_Format_BINorBCD, &RTC_DateStructure);
	RTC_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);
}

/**
  * @brief  显示时间和日期
  * @param  无
  * @retval 无
  */
void RTC_TimeAndDate_Show(void)
{
	uint8_t Rtctmp=0;
//	char LCDTemp[100];
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;
	
	
	while(1)
	{
		// 获取日历
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	  RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
		
		// 每秒打印一次
  	if(Rtctmp != RTC_TimeStructure.RTC_Seconds)
    {
						
			// 打印日期
      printf("The Date :  Y:20%0.2d - M:%0.2d - D:%0.2d - W:%0.2d\r\n", 
			RTC_DateStructure.RTC_Year,
			RTC_DateStructure.RTC_Month, 
			RTC_DateStructure.RTC_Date,
			RTC_DateStructure.RTC_WeekDay);
		
			//液晶显示日期
			//先把要显示的数据用sprintf函数转换为字符串，然后才能用液晶显示函数显示
//			sprintf(LCDTemp,"The Date :  Y:20%0.2d - M:%0.2d - D:%0.2d - W:%0.2d", 
//			RTC_DateStructure.RTC_Year,
//			RTC_DateStructure.RTC_Month, 
//			RTC_DateStructure.RTC_Date,
//			RTC_DateStructure.RTC_WeekDay);

//			LCD_DisplayStringLineEx(10,50,48,48,(uint8_t *)LCDTemp,0); 
			
			// 打印时间
      printf("The Time :  %0.2d:%0.2d:%0.2d \r\n\r\n", 
			RTC_TimeStructure.RTC_Hours, 
			RTC_TimeStructure.RTC_Minutes, 
			RTC_TimeStructure.RTC_Seconds);
			
			//液晶显示时间
//			sprintf(LCDTemp,"The Time :  %0.2d:%0.2d:%0.2d", 
//			RTC_TimeStructure.RTC_Hours, 
//			RTC_TimeStructure.RTC_Minutes, 
//			RTC_TimeStructure.RTC_Seconds);

//			LCD_DisplayStringLineEx(10,100,48,48,(uint8_t *)LCDTemp,0);
			
      (void)RTC->DR;
    }
    Rtctmp = RTC_TimeStructure.RTC_Seconds;
	}	
}

/**
  * @brief  RTC配置：选择RTC时钟源，设置RTC_CLK的分频系数
  * @param  无
  * @retval 无
  */
void RTC_CLK_Config(void)
{  
	RTC_InitTypeDef RTC_InitStructure;
	
	/*使能 PWR 时钟*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  /* PWR_CR:DBF置1，使能RTC、RTC备份寄存器和备份SRAM的访问 */
  PWR_BackupAccessCmd(ENABLE);

#if defined (RTC_CLOCK_SOURCE_LSI) 
  /* 使用LSI作为RTC时钟源会有误差 
	 * 默认选择LSE作为RTC的时钟源
	 */
  /* 使能LSI */ 
  RCC_LSICmd(ENABLE);
  /* 等待LSI稳定 */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {
  }
  /* 选择LSI做为RTC的时钟源 */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

#elif defined (RTC_CLOCK_SOURCE_LSE)

  /* 使能LSE */ 
  RCC_LSEConfig(RCC_LSE_ON);
   /* 等待LSE稳定 */   
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
  }
  /* 选择LSE做为RTC的时钟源 */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);    

#endif /* RTC_CLOCK_SOURCE_LSI */

  /* 使能RTC时钟 */
  RCC_RTCCLKCmd(ENABLE);

  /* 等待 RTC APB 寄存器同步 */
  RTC_WaitForSynchro();
   
/*=====================初始化同步/异步预分频器的值======================*/
	/* 驱动日历的时钟ck_spare = LSE/[(255+1)*(127+1)] = 1HZ */
	
	/* 设置异步预分频器的值 */
	RTC_InitStructure.RTC_AsynchPrediv = ASYNCHPREDIV;
	/* 设置同步预分频器的值 */
	RTC_InitStructure.RTC_SynchPrediv = SYNCHPREDIV;	
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; 
	/* 用RTC_InitStructure的内容初始化RTC寄存器 */
	if (RTC_Init(&RTC_InitStructure) == ERROR)
	{
		printf("\n\r RTC 时钟初始化失败 \r\n");
	}	
}

/**
  * @brief  RTC配置：选择RTC时钟源，设置RTC_CLK的分频系数
  * @param  无
  * @retval 无
  */
#define LSE_STARTUP_TIMEOUT     ((uint16_t)0x05000)
void RTC_CLK_Config_Backup(void)
{  
  __IO uint16_t StartUpCounter = 0;
	FlagStatus LSEStatus = RESET;	
	RTC_InitTypeDef RTC_InitStructure;
	
	/* 使能 PWR 时钟 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  /* PWR_CR:DBF置1，使能RTC、RTC备份寄存器和备份SRAM的访问 */
  PWR_BackupAccessCmd(ENABLE);
	
/*=========================选择RTC时钟源==============================*/
/* 默认使用LSE，如果LSE出故障则使用LSI */
  /* 使能LSE */
  RCC_LSEConfig(RCC_LSE_ON);	
	
	/* 等待LSE启动稳定，如果超时则退出 */
  do
  {
    LSEStatus = RCC_GetFlagStatus(RCC_FLAG_LSERDY);
    StartUpCounter++;
  }while((LSEStatus == RESET) && (StartUpCounter != LSE_STARTUP_TIMEOUT));
	
	
	if(LSEStatus == SET )
  {
		printf("\n\r LSE 启动成功 \r\n");
		/* 选择LSE作为RTC的时钟源 */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  }
	else
	{
		printf("\n\r LSE 故障，转为使用LSI \r\n");
		
		/* 使能LSI */	
		RCC_LSICmd(ENABLE);
		/* 等待LSI稳定 */ 
		while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		{			
		}
		
		printf("\n\r LSI 启动成功 \r\n");
		/* 选择LSI作为RTC的时钟源 */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	}
	
  /* 使能 RTC 时钟 */
  RCC_RTCCLKCmd(ENABLE);
  /* 等待 RTC APB 寄存器同步 */
  RTC_WaitForSynchro();

/*=====================初始化同步/异步预分频器的值======================*/
	/* 驱动日历的时钟ck_spare = LSE/[(255+1)*(127+1)] = 1HZ */
	
	/* 设置异步预分频器的值为127 */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	/* 设置同步预分频器的值为255 */
	RTC_InitStructure.RTC_SynchPrediv = 0xFF;	
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; 
	/* 用RTC_InitStructure的内容初始化RTC寄存器 */
	if (RTC_Init(&RTC_InitStructure) == ERROR)
	{
		printf("\n\r RTC 时钟初始化失败 \r\n");
	}	
}


/**********************************END OF FILE*************************************/
