#include "dwt.h"

static uint32_t cpuclkfeq;     //用于保存cpu运行频率，可运行时动态修改

//初始化延时系统，参数为CPU频率

void DelayInit(uint32_t clk)
{

    cpuclkfeq = clk;
//打开CYCCNT功能,并把计数器清零，最后打开计数器对cpu时钟进行向上计数

    DEM_CR         |=  DEM_CR_TRCENA; 

    DWT_CYCCNT      = 0u;    //根据需要如果调试，或其他程序要使用CYCCNT时注释掉，否则可直接清零 

    DWT_CR         |= DWT_CR_CYCCNTENA;

}

//延时函数，参数为需要延时的微秒数
void Delayus(uint32_t usec)

{
     uint32_t startts,endts,ts;
     UINT32 uwIntSave;

  //保存进入函数时的计数器值

     startts = DWT_CYCCNT;

     ts =  usec * (cpuclkfeq /(1000*1000));        //计算达到所需延时值的cpu时钟数,^-^如果想要更精确此处可以减去运行前面代码所需的时钟数。

     endts = startts + ts;           //计算达到所需延时时间的DWT_CYCCNT计数值，超过32bit所能表达的最大值2的32次方-1是自动绕回丢弃进位
     uwIntSave=LOS_IntLock();
      if(endts > startts)            //判断是否跨越最大值边界

      {

          while(DWT_CYCCNT < endts);        //等到计数到所需延时值的cpu时钟数值

       }

       else

      {

           while(DWT_CYCCNT > endts);       //等待跨域32bit的最大值，2的32次方-1

           while(DWT_CYCCNT < endts);        //等到计数到所需延时值的cpu时钟数值

      }
    (VOID)LOS_IntRestore(uwIntSave);

}

void Delay10ms(__IO u32 nTime)
{
    Delayus(1000*nTime);
}
