/**
 * Init USART2 for Modbus RS485
 */

#ifndef __RS485_USART_H
#define	__RS485_USART_H

/** std lib */
#include "stm32f4xx.h"
#include <stdio.h>

/** USART2 HW. */
//#define USART1_DR_Base  0x40013804		// 0x40013800 + 0x04 = 0x40013804
//#define SENDBUFF_SIZE   5000
#define RS485_USART                             USART2
#define RS485_USART_CLK                         RCC_APB1Periph_USART2
#define RS485_USART_BAUDRATE                    115200

#define RS485_USART_RX_GPIO_PORT                GPIOD
#define RS485_USART_RX_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define RS485_USART_RX_PIN                      GPIO_Pin_6
#define RS485_USART_RX_AF                       GPIO_AF_USART2
#define RS485_USART_RX_SOURCE                   GPIO_PinSource6

#define RS485_USART_TX_GPIO_PORT                GPIOD
#define RS485_USART_TX_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define RS485_USART_TX_PIN                      GPIO_Pin_5
#define RS485_USART_TX_AF                       GPIO_AF_USART2
#define RS485_USART_TX_SOURCE                   GPIO_PinSource5

//收发管脚
#define RS485_GPIO_PIN                          GPIO_Pin_11                 
#define RS485_GPIO_PORT                         GPIOD                     
#define RS485_GPIO_CLK                          RCC_AHB1Periph_GPIOD

#define	digitalHi(p,i)			                {p->BSRRL=i;}		//设置为高电平	-- reference bsp_led.h
#define digitalLo(p,i)			                {p->BSRRH=i;}		//输出低电平
#define RS485_TX_ON_RX_OFF    	                digitalHi(RS485_GPIO_PORT,RS485_GPIO_PIN)
#define RS485_RX_ON_TX_OFF		                digitalLo(RS485_GPIO_PORT,RS485_GPIO_PIN)


/** RS485 */
#define NUM_TX_BUFF	128			//定义发送缓冲区大小
#define NUM_RX_BUFF	128			//定义接受缓冲区大小

enum{
	MODBUS_03		= 0x03,		//03号 读寄存器
};

enum{
	MODBUS_03_SUB_06= 0x06,		//03号 读寄存器的子类别 - 电动机运行状态
	MODBUS_03_SUB_07= 0x07      //	                    - 电动机的模拟量
};


/** bool */
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/** modbus 固化对侧设备地址  */
#define MODBUS_ADDRESS_SLAVE 0x01

/** Modbus Struct */
typedef struct {	
	uint16_t address;               //通讯地址	0~255
	
	/**接受**/
	uint8_t RxBuff[NUM_RX_BUFF];	//接受缓冲区
	uint16_t RxLen;					//接受长度
	uint16_t RxCount;				//接受计数
	
	/**发送**/
	uint8_t TxBuff[NUM_TX_BUFF];	//发送缓冲区
	uint16_t TxLen;					//发送长度
	uint16_t TxCount;				//发送计数
	
	/**是否有新报文未处理标志**/
	uint8_t bNew;					//TRUE: 有新的一包数据需要处理
	/**是否有数据正在发送标志**/
	uint8_t bSend;					//TRUE: 发送数据
 	
// 	volatile uint32 RxTimer;		//接受时间计数器
//	volatile uint32 TxTimer;		//发送超时计数器
//	volatile uint32 OpTimer;		//操作超时计时器
//	volatile uint32 ExTimer;		//扩展计时器，用于某些特殊功能
	
	//modbus 私有数据类型 - 接收
	uint16_t modbusRxCRC;			//ModbusCRC校验
	uint8_t  modbusCode;			//报文特征码	
}TBuffModbus;

//电动机状态
enum{
	STATUS_STOP			=0x00,	//停止态
	STATUS_STARTING,			//启动态
	STATUS_RUN					//运行态
};

//电动机转向 正向/反向
enum{
	STATUS_POSITIVE,			//正向
	STATUS_NEGATIVE 			//反向
};

typedef struct{
	uint8_t motor_status_now:3;	// 电机当前状态 - 每次 modbus 上送的电动机 state 报文都会更新这个值,
	uint8_t motor_status_last:3;// 电机的上一个状态 - mqtt中会比较 _now 与 _last 是否不同, 若是不同则上送 mqtt 电机状态发生变化
	uint8_t motor_dir:2;		// 电动机转向 正向/反向
}S_Status;

// 电动机状态 - Modbus 功能码 0x03  分类码 0x06;
typedef union{
	uint8_t ui;
	S_Status s;
}TStatus;

//电动机测量值 - Modbus 功能码 0x03  分类码 0x07;
typedef struct{
	/***三相电流 - 测量量***/
	uint16_t Meas_Ia;		
}TMeasure;

typedef struct {    
	TStatus Status;
    TMeasure Measure;
}TMotor;

extern TMotor stMotor; /** mqtt 处理时需要的 global 变量 */

/*------------------------------------------------------------------*/
/** 定义一些用于维持 OS 时间的变量, 这些功能实现后可以 用于函数 HAL_UptimeMs() 等 */
/** 同时还需要结合 OS tick 切换中断函数, 因为该函数这里每 1ms 中断 1 次 */
/**
 *	日期时间，精确到毫秒
 *	年的表示方式为两位，比如2007年，就表示为y=7
 */
typedef struct {
	uint8_t y;	    //年
	uint8_t mon;	//月
	uint8_t d;	    //日
	uint8_t h;	    //时
	uint8_t min;	//分
	uint8_t s;	    //秒
	uint16_t ms;	//毫秒
}TDateTime;

extern TDateTime stNowTime;

uint8_t CheckOSTime(TDateTime * pTime);
void InitOSTime(TDateTime * pTime); 
inline void OSTimeAddSecond(TDateTime * pTime);

/*------------------------------------------------------------------*/
/** Function API - Motor State */
/** 读取/设置电机状态 */
#define GetMotorStatusNow() (stMotor.Status.s.motor_status_now)
#define SetMotorStatusNow(status) (stMotor.Status.s.motor_status_now=(status))
#define GetMotorStatusLast() (stMotor.Status.s.motor_status_last)
#define SetMotorStatusLast(status) (stMotor.Status.s.motor_status_last=(status))

/** Function API - Modbus */
void RS485_USART2_Config(void);
void Modbus_OnSend(TBuffModbus *pBuff);
void Moubus_OnRecvive(TBuffModbus *pBuff);
void ModbusProc_InTask(TBuffModbus *pBuff);

#endif /* __USART2_H */
