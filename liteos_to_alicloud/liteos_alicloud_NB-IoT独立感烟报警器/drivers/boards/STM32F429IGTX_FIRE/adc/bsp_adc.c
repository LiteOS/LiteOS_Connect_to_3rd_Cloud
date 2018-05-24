#include "bsp_adc.h"

uint16_t ADC_ConvertedValue;

void Rheostat_ADC_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能 GPIO 时钟
    RCC_AHB1PeriphClockCmd(RHEOSTAT_ADC_GPIO_CLK, ENABLE);

    // 配置 IO
    GPIO_InitStructure.GPIO_Pin = RHEOSTAT_ADC_GPIO_PIN;
    // 配置为模拟输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    // 不上拉不下拉
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(RHEOSTAT_ADC_GPIO_PORT, &GPIO_InitStructure);
}

void Rheostat_ADC_Mode_Config(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;

    // 开启 ADC 时钟
    RCC_APB2PeriphClockCmd(RHEOSTAT_ADC_CLK , ENABLE);

    // -------------------ADC Common 结构体 参数 初始化--------------------
    // 独立 ADC 模式
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    // 时钟为 fpclk x 分频
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
    // 禁止 DMA 直接访问模式
    ADC_CommonInitStructure.ADC_DMAAccessMode=ADC_DMAAccessMode_Disabled;
    // 采样时间间隔
    ADC_CommonInitStructure.ADC_TwoSamplingDelay=
    ADC_TwoSamplingDelay_10Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    // -------------------ADC Init 结构体 参数 初始化---------------------
    // ADC 分辨率
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    // 禁止扫描模式，多通道采集才需要
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    // 连续转换
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    //禁止外部边沿触发
    ADC_InitStructure.ADC_ExternalTrigConvEdge =
    ADC_ExternalTrigConvEdge_None;
    //使用软件触发，外部触发不用配置，注释掉即可
    //ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_T1_CC1;
    //数据右对齐
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    //转换通道 1 个
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(RHEOSTAT_ADC, &ADC_InitStructure);
    //------------------------------------------------------------------
    // 配置 ADC 通道转换顺序为 1，第一个转换，采样时间为 56 个时钟周期
    ADC_RegularChannelConfig(RHEOSTAT_ADC, RHEOSTAT_ADC_CHANNEL,
    1, ADC_SampleTime_56Cycles);

    // ADC 转换结束产生中断，在中断服务程序中读取转换值
    //ADC_ITConfig(RHEOSTAT_ADC, ADC_IT_EOC, ENABLE);
    // 使能 ADC
    //ADC_Cmd(RHEOSTAT_ADC, ENABLE);
    //开始 adc 转换，软件触发
    //ADC_SoftwareStartConv(RHEOSTAT_ADC);
}

uint16_t Rheostat_ADC_StartConv(void)
{
    // 使能 ADC
    ADC_Cmd(RHEOSTAT_ADC, ENABLE);
    //开始 adc 转换，软件触发
    ADC_SoftwareStartConv(RHEOSTAT_ADC);

    while(ADC_GetFlagStatus(RHEOSTAT_ADC,ADC_FLAG_EOC) == RESET){
    }
    
    ADC_ConvertedValue = ADC_GetConversionValue(RHEOSTAT_ADC);

    return ADC_ConvertedValue;
}

void Rheostat_ADC_EndConv(void)
{
    ADC_Cmd(RHEOSTAT_ADC, DISABLE);
}

void Rheostat_ADC_Init()
{
    Rheostat_ADC_GPIO_Config();
    Rheostat_ADC_Mode_Config();
}
