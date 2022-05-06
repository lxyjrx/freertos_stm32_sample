#include "FreeRTOS.h"
#include "task.h"

#include "adc.h"
#include "tim.h"

#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_dma.h"

#include <stdio.h>

static uint16_t ADC_ConvertedValue[SAMPLE_NUM][CHANNEL_NUM];




void ADC_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
    
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能dma外设时钟
	
    DMA_DeInit(DMA1_Channel1);  //将DMA的通道1寄存器重设为缺省值
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;  //DMA外设 ADC基地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_ConvertedValue;  //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;      //内存作为数据传输的目的地
    DMA_InitStructure.DMA_BufferSize = SAMPLE_NUM*CHANNEL_NUM;    //DMA通道的DMA缓存大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;    //内存地址递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //DMA通道的DMA缓存的大小
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;   //DMA通道的DMA缓存的大小
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//  DMA_Mode_Normal;  //工作在循环缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有高优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   //DMA通道x没有设置为内存到内存传输
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);   //根据DMA_InitStruct中指定的参数初始化DMA的通道
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); //DMA_IT_TC

    DMA_Cmd(DMA1_Channel1,ENABLE);
 
    /* Enable DMA channel1 IRQ Channel */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
}

static void ADC1_Reset(void)
{
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
}

void ADC1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    //PC0-通道10 ,PC1-通道11 ,PC2-通道12 ,PC3-通道13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    ADC_DMA_Config();
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); //设置ADC CLK为12MHz
    
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  //ADC1和ADC2工作在独立模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;       //扫描多通道
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//ENABLE;      //连续转换
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO; //选择TIM3 trigo触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;     //转换结果右对齐
    ADC_InitStructure.ADC_NbrOfChannel = CHANNEL_NUM;      //通道数目
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_55Cycles5);    //通道，转换次序，转换时间
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_55Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_55Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_55Cycles5);
    
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
   
    ADC_ResetCalibration(ADC1);   //使能复位校准
    while(ADC_GetResetCalibrationStatus(ADC1));   //等待复位校准结束
    ADC_StartCalibration(ADC1);     //开始AD校准
    while(ADC_GetCalibrationStatus(ADC1));   //等待校准结束
    
    //ADC_SoftwareStartConvCmd(ADC1, ENABLE);  //使能ADC的软件转换启动功能
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);  //使能外部触发
}

uint16_t read_adc_average_value(uint16_t channel)
{
    uint8_t i;
    uint32_t sum;
    
    for(i=0; i<SAMPLE_NUM; i++)
    {
        sum += ADC_ConvertedValue[i][channel];
    }
    
    return (sum / SAMPLE_NUM);
}

#define ADC_INPUT_VOLTAGE_MAX 3300  //unit:mV
uint16_t convert_adc_value_to_voltage(uint16_t adc_value)
{
    uint16_t voltage;   //输入模拟电压为0~3.3v

    voltage = (ADC_INPUT_VOLTAGE_MAX>>12)* adc_value;
    
    return voltage;
}

uint16_t convert_voltage_to_temperature(uint16_t voltage)
{
    uint16_t temp;   //输入模拟电压为0~3.3v

    temp = voltage;
    
    return temp;
}

/* 存储将在传输完成时通知的任务的句柄 */
static TaskHandle_t xTaskToNotify = NULL;

/* 目标任务的任务通知数组中要使用的索引。 */
const UBaseType_t xArrayIndex = 0;

void DMA1_Channel1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearITPendingBit(DMA1_IT_GL1); //清除全部中断标志
        
        /* 此时xTaskToNotify不应该是NULL，因为传输正在进行中。 */
        configASSERT( xTaskToNotify != NULL );
        
        /* 通知任务传输已完成。 */
        vTaskNotifyGiveIndexedFromISR( xTaskToNotify,
                                   xArrayIndex,
                                   &xHigherPriorityTaskWoken );

        ADC1_Reset();  //重新配置ADC

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* 发起传输的任务随后进入阻塞状态（因此不消耗任何CPU时间）来等待它完成。 */
void start_adc_sample(void)
{
	uint32_t ulNotificationValue;
	//const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );

    /* 存储调用任务的句柄。 */
    xTaskToNotify = xTaskGetCurrentTaskHandle();
    
    /* 等待传输完成的通知。注意，第一个参数是pdTRUE，它的作用是将任务
    的通知值清除回0，使通知值像一个二进制信号量(而不是计数信号量)。 */
    ulNotificationValue = ulTaskNotifyTakeIndexed( xArrayIndex,
                                                   pdTRUE,
                                                   portMAX_DELAY );

    if( ulNotificationValue == 1 )
    {
        /* 传输如期结束。 */
        printf("adc work done!!!\n");
    }
    else
    {
        /* 调用ulTaskNotifyTake()超时。 */
    }
}

