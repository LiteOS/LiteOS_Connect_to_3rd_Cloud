/**
  ******************************************************************************
  * @file    bsp_dht11.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   温湿度传感器应用函数接口
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F429 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
#include "bsp_dht11.h"
#include "utils.h"
#include "stm32f4xx.h"
#include "bsp_led.h" 
#include "./Bsp/usart/bsp_debug_usart.h"
#include "./Bsp/systick/bsp_SysTick.h"
#include "dwt.h"
/*
 * 函数名：DHT11_GPIO_Config
 * 描述  ：配置DHT11用到的I/O口
 * 输入  ：无
 * 输出  ：无
 */
void DHT11_GPIO_Config(void)
{		
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*开启DHT11_PORT的外设时钟*/
	RCC_AHB1PeriphClockCmd(DHT11_CLK, ENABLE); 

	/*选择要控制的DHT11_PORT引脚*/															   
  	GPIO_InitStructure.GPIO_Pin = DHT11_PIN;	

	/*设置引脚模式为通用推挽输出*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
  
  /*设置引脚的输出类型为推挽输出*/
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  
  /*设置引脚为上拉模式*/
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
	/*设置引脚速率为50MHz */   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*调用库函数，初始化DHT11_PORT*/
  GPIO_Init(DHT11_PORT, &GPIO_InitStructure); 
}

/*
 * 函数名：DHT11_Mode_IPU
 * 描述  ：使DHT11-DATA引脚变为上拉输入模式
 * 输入  ：无
 * 输出  ：无
 */
static void DHT11_Mode_IPU(void)
{
 	  GPIO_InitTypeDef GPIO_InitStructure;

	  	/*选择要控制的DHT11_PORT引脚*/	
	  GPIO_InitStructure.GPIO_Pin = DHT11_PIN;

	   /*设置引脚模式为浮空输入模式*/ 
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN ; 
  
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
  	/*设置引脚速率为50MHz */   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  
	  /*调用库函数，初始化DHT11_PORT*/
	  GPIO_Init(DHT11_PORT, &GPIO_InitStructure);	 
}

/*
 * 函数名：DHT11_Mode_Out_PP
 * 描述  ：使DHT11-DATA引脚变为推挽输出模式
 * 输入  ：无
 * 输出  ：无
 */
static void DHT11_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;

	/*选择要控制的DHT11_PORT引脚*/															   
  GPIO_InitStructure.GPIO_Pin = DHT11_PIN;	

	/*设置引脚模式为通用推挽输出*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
  
  /*设置引脚的输出类型为推挽输出*/
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  
  /*设置引脚为上拉模式*/
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
	/*设置引脚速率为50MHz */   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*调用库函数，初始化DHT11_PORT*/
  	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);	 	 
}

/* 
 * 从DHT11读取一个字节，MSB先行
 */
static uint8_t Read_Byte(void)
{
	uint8_t i, temp=0;

	for(i=0;i<8;i++)    
	{	 
		/*每bit以50us低电平标置开始，轮询直到从机发出 的50us 低电平 结束*/  
		while(DHT11_DATA_IN()==Bit_RESET);

		/*DHT11 以26~28us的高电平表示“0”，以70us高电平表示“1”，
		 *通过检测 x us后的电平即可区别这两个状 ，x 即下面的延时 
		 */
		Delayus(40); //延时x us 这个延时需要大于数据0持续的时间即可	   	  

		if(DHT11_DATA_IN()==Bit_SET)/* x us后仍为高电平表示数据“1” */
		{
			/* 等待数据1的高电平结束 */
			while(DHT11_DATA_IN()==Bit_SET);

			temp|=(uint8_t)(0x01<<(7-i));  //把第7-i位置1，MSB先行 
		}
		else	 // x us后为低电平表示数据“0”
		{			   
			temp&=(uint8_t)~(0x01<<(7-i)); //把第7-i位置0，MSB先行
		}
	}
	return temp;
}
/*
 * 一次完整的数据传输为40bit，高位先出
 * 8bit 湿度整数 + 8bit 湿度小数 + 8bit 温度整数 + 8bit 温度小数 + 8bit 校验和 
 */
uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{  
  uint16_t count;
	/*输出模式*/
	DHT11_Mode_Out_PP();
	/*主机拉低*/
	DHT11_DATA_OUT(DHT11_LOW);
	/*延时18ms*/
	Delayus(20000);

	/*总线拉高 主机延时30us*/
	DHT11_DATA_OUT(DHT11_HIGH); 

	Delayus(30);   //延时30us

	/*主机设为输入 判断从机响应信号*/ 
	DHT11_Mode_IPU();

	/*判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行*/   
	if(DHT11_DATA_IN()==Bit_RESET)     
	{
    count=0;
		/*轮询直到从机发出 的80us 低电平 响应信号结束*/  
		while(DHT11_DATA_IN()==Bit_RESET)
    {
      count++;
      if(count>1000)  return 0;
      Delayus(10); 
    }    
    
    count=0;
		/*轮询直到从机发出的 80us 高电平 标置信号结束*/
		while(DHT11_DATA_IN()==Bit_SET)
    {
      count++;
      if(count>1000)  return 0;
      Delayus(10); 
    }  
		/*开始接收数据*/   
		DHT11_Data->humi_int= Read_Byte();

		DHT11_Data->humi_deci= Read_Byte();

		DHT11_Data->temp_int= Read_Byte();

		DHT11_Data->temp_deci= Read_Byte();

		DHT11_Data->check_sum= Read_Byte();

		/*读取结束，引脚改为输出模式*/
		DHT11_Mode_Out_PP();
		/*主机拉高*/
		DHT11_DATA_OUT(DHT11_HIGH);

		/*检查读取的数据是否正确*/
		if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int+ DHT11_Data->temp_deci)
			return 1;
		else 
			return 0;
	}
	else
	{		
		return 0;
	}   
}




/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
DHT11_Data_TypeDef DHT11_Data1;
extern int temperature1;
extern int humidity1;
char post_data1[255]={0};
extern char post_data2[255];//在tcp_echoclient.c的MCU_TO_TCP()函数中调用
int count=0;
//修改自己的设备ID和APIkey
char post_head[]={"POST /devices/29702274/datapoints?type=3 HTTP/1.1\r\napi-key: TnWg0VdW6kIhU9=6I9AewR=TxKQ=\r\nHost: api.heclouds.com\r\nContent-Length:"};
void mainDHT11(void)
{	
		/*调用DHT11_Read_TempAndHumidity读取温湿度，若成功则输出该信息*/
		if( Read_DHT11 ( & DHT11_Data1 ) == SUCCESS)
		{
			LED33=~LED33;
			temperature1=DHT11_Data1.temp_int;
			humidity1=DHT11_Data1.humi_int;
			sprintf(post_data1,"{\"temperature\":%d,\"humidity\":%d,\"Count\":%d}",temperature1,humidity1,count++);//,,\"LED1\":%d(int)LED33);
			sprintf(post_data2,"%s%d\r\n\r\n%s",post_head,strlen(post_data1),post_data1);
			printf("\r\n读取DHT11成功!\r\n\r\n%s,len:[%d]",post_data1,strlen(post_data1));
			if(count==100000)count=0;//
		}		
		else
		{			
		  printf("Read DHT11 ERROR!\r\n");
	  
		}
}





