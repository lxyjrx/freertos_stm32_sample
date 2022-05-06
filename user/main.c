#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "stm32f10x.h"
#include "adc.h"
#include "tim.h"
#include "led.h"

//int temp = 1; //测试使用
//int temp2 = 1;    //测试使用
//int temp3 = 1;  //测试使用
TaskHandle_t task1_handle;

extern int printf(const char *format, ...);
//void vApplicationTickHook( void )
//{
//    temp3 ^= 0x01;
//    if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_0))
//    {
//        GPIO_ResetBits(GPIOA, GPIO_Pin_0);
//    }
//    else
//    {
//        GPIO_SetBits(GPIOA, GPIO_Pin_0);
//    }
//}

//void vApplicationIdleHook( void ) 
//{ 
//    //temp3 ^= 0x01;
//}

#define MAX_TASK_NUM        5
TaskStatus_t pxTaskStatusArray[MAX_TASK_NUM];

/*获取OS任务信息*/
void get_task_state(void)
{
    const char task_state[]={'r','R','B','S','D'};
    volatile UBaseType_t uxArraySize, x;
     unsigned portSHORT ulTotalRunTime,ulStatsAsPercentage;
 
    /* 获取任务总数目 */
    uxArraySize = uxTaskGetNumberOfTasks();
   if(uxArraySize > MAX_TASK_NUM)
    {
        printf("当前任务数量过多！\n");
    }
 
    /*获取每个任务的状态信息 */
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime );

    printf("任务名        状态       ID       优先级       堆栈        CPU使用率\n");
 
    /* 避免除零错误 */
    if( ulTotalRunTime > 0 )
    {
        /* 将获得的每一个任务状态信息部分的转化为程序员容易识别的字符串格式 */
        for( x = 0; x < uxArraySize; x++ )
        {
            char tmp[128];
           
            /* 计算任务运行时间与总运行时间的百分比。*/
            ulStatsAsPercentage =(uint16_t)(pxTaskStatusArray[ x ].ulRunTimeCounter)*100 / ulTotalRunTime;
 
            if( ulStatsAsPercentage > 0UL )
            {
 
               sprintf(tmp,"%-15s%-10c%-10d%-12d%-12d%d%%",pxTaskStatusArray[ x].pcTaskName,task_state[pxTaskStatusArray[ x ].eCurrentState],
                                                                       pxTaskStatusArray[ x ].xTaskNumber,pxTaskStatusArray[ x].uxCurrentPriority,
                                                                       pxTaskStatusArray[ x ].usStackHighWaterMark,ulStatsAsPercentage);
            }
            else
            {
                /* 任务运行时间不足总运行时间的1%*/
                sprintf(tmp,"%-15s%-10c%-10d%-12d%-12dt<1%%",pxTaskStatusArray[x ].pcTaskName,task_state[pxTaskStatusArray[ x ].eCurrentState],
                                                                       pxTaskStatusArray[ x ].xTaskNumber,pxTaskStatusArray[ x].uxCurrentPriority,
                                                                       pxTaskStatusArray[ x ].usStackHighWaterMark);               
            }
           printf("%s\n",tmp);
        }
    }
    printf("任务状态:   r-运行  R-就绪  B-阻塞  S-挂起  D-删除\n");
   
}

void vTask1( void *pvParameters ) 
{   
    volatile int i = 0, j = 0;
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    /* 和大多数任务一样，本任务也处于一个死循环中 */ 
    for( ;; ) 
    {
        //temp ^= 0x01;
        
        //printf( "[%s] is running\n", (char *)pvParameters);
        for(i=0; i<=200; i++)
        {
            for(j=0; j<=100; j++)
                ;
        }

        //延迟1000ms
        vTaskDelayUntil(&xLastWakeTime, 50 / portTICK_RATE_MS);
        //vTaskDelay(100 / portTICK_RATE_MS);
    } 
}

//任务2负责获取任务状态信息并格式化打印输出到调试窗口
void monitor_task( void *pvParameters ) 
{   
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    /* 和大多数任务一样，本任务也处于一个死循环中 */ 
    for( ;; ) 
    { 
        //temp2 ^= 0x01;
        get_task_state();
        //printf( "[%s] is running\n", (char *)pvParameters);
        
        //延迟1000ms
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);
        //vTaskDelay(1000 / portTICK_RATE_MS);
    } 
}

#define ADC_CHANNEL_NUM 10  //adc通道的编号
#define TIMER_NUM_FOR_ADC 3 //浣跨敤timer3
//ADC任务,负责处理ADC采样数据,得到温度值
void ADC_Task( void *pvParameters ) 
{   
    uint16_t adc_value;
    uint16_t voltage;
    uint16_t fuser_temperature;

    if(TimerStart(TIMER_NUM_FOR_ADC) < 0)
    {
        printf("start timer failed!!\n");
    }
    
    /* 和大多数任务一样，本任务也处于一个死循环中 */ 
    for( ;; ) 
    {   
        //配置ADC,配置DMA
        ADC1_Config();
     
        //启动ADC开始工作，实际上adc是由timer触发采样
        start_adc_sample();
        
        //获取ADC采样值
        adc_value = read_adc_average_value(ADC_CHANNEL_NUM);
        //把adc采样值转换为电压值
        voltage = convert_adc_value_to_voltage(adc_value);
        //把adc电压值转换为温度值
        fuser_temperature = convert_voltage_to_temperature(voltage);
        printf("current temperature:%d\n", fuser_temperature);
        //发送温度值给下一个任务(可以发送任务通知)
    } 
}


int main(void)
{
    //Hardware init
    //设置NVIC中断优先级分组为组4
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    //硬件模块初始化
    //led模块初始化
    //led_init();
    //TIMER模块初始化
    TimerInit(); //定时时间设为1秒
    //
    //OS init
    //......
    
    //task init
    xTaskCreate( vTask1, "task1", 150, ( void * ) "task1", 1, &task1_handle );
    xTaskCreate( monitor_task, "monitor task", 150, ( void * ) "monitor task", 1, NULL );
    xTaskCreate( ADC_Task, "ADC Task", 150, ( void * ) "ADC Task", 2, NULL );
    
    //start kernel
    vTaskStartScheduler();
    while(1)
    {
        ;
    }
}
