#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "stm32f10x.h"
#include "adc.h"
#include "tim.h"
#include "led.h"

//int temp = 1; //����ʹ��
//int temp2 = 1;    //����ʹ��
//int temp3 = 1;  //����ʹ��
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

/*��ȡOS������Ϣ*/
void get_task_state(void)
{
    const char task_state[]={'r','R','B','S','D'};
    volatile UBaseType_t uxArraySize, x;
     unsigned portSHORT ulTotalRunTime,ulStatsAsPercentage;
 
    /* ��ȡ��������Ŀ */
    uxArraySize = uxTaskGetNumberOfTasks();
   if(uxArraySize > MAX_TASK_NUM)
    {
        printf("��ǰ�����������࣡\n");
    }
 
    /*��ȡÿ�������״̬��Ϣ */
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime );

    printf("������        ״̬       ID       ���ȼ�       ��ջ        CPUʹ����\n");
 
    /* ���������� */
    if( ulTotalRunTime > 0 )
    {
        /* ����õ�ÿһ������״̬��Ϣ���ֵ�ת��Ϊ����Ա����ʶ����ַ�����ʽ */
        for( x = 0; x < uxArraySize; x++ )
        {
            char tmp[128];
           
            /* ������������ʱ����������ʱ��İٷֱȡ�*/
            ulStatsAsPercentage =(uint16_t)(pxTaskStatusArray[ x ].ulRunTimeCounter)*100 / ulTotalRunTime;
 
            if( ulStatsAsPercentage > 0UL )
            {
 
               sprintf(tmp,"%-15s%-10c%-10d%-12d%-12d%d%%",pxTaskStatusArray[ x].pcTaskName,task_state[pxTaskStatusArray[ x ].eCurrentState],
                                                                       pxTaskStatusArray[ x ].xTaskNumber,pxTaskStatusArray[ x].uxCurrentPriority,
                                                                       pxTaskStatusArray[ x ].usStackHighWaterMark,ulStatsAsPercentage);
            }
            else
            {
                /* ��������ʱ�䲻��������ʱ���1%*/
                sprintf(tmp,"%-15s%-10c%-10d%-12d%-12dt<1%%",pxTaskStatusArray[x ].pcTaskName,task_state[pxTaskStatusArray[ x ].eCurrentState],
                                                                       pxTaskStatusArray[ x ].xTaskNumber,pxTaskStatusArray[ x].uxCurrentPriority,
                                                                       pxTaskStatusArray[ x ].usStackHighWaterMark);               
            }
           printf("%s\n",tmp);
        }
    }
    printf("����״̬:   r-����  R-����  B-����  S-����  D-ɾ��\n");
   
}

void vTask1( void *pvParameters ) 
{   
    volatile int i = 0, j = 0;
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    /* �ʹ��������һ����������Ҳ����һ����ѭ���� */ 
    for( ;; ) 
    {
        //temp ^= 0x01;
        
        //printf( "[%s] is running\n", (char *)pvParameters);
        for(i=0; i<=200; i++)
        {
            for(j=0; j<=100; j++)
                ;
        }

        //�ӳ�1000ms
        vTaskDelayUntil(&xLastWakeTime, 50 / portTICK_RATE_MS);
        //vTaskDelay(100 / portTICK_RATE_MS);
    } 
}

//����2�����ȡ����״̬��Ϣ����ʽ����ӡ��������Դ���
void monitor_task( void *pvParameters ) 
{   
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    /* �ʹ��������һ����������Ҳ����һ����ѭ���� */ 
    for( ;; ) 
    { 
        //temp2 ^= 0x01;
        get_task_state();
        //printf( "[%s] is running\n", (char *)pvParameters);
        
        //�ӳ�1000ms
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);
        //vTaskDelay(1000 / portTICK_RATE_MS);
    } 
}

#define ADC_CHANNEL_NUM 10  //adcͨ���ı��
#define TIMER_NUM_FOR_ADC 3 //使用timer3
//ADC����,������ADC��������,�õ��¶�ֵ
void ADC_Task( void *pvParameters ) 
{   
    uint16_t adc_value;
    uint16_t voltage;
    uint16_t fuser_temperature;

    if(TimerStart(TIMER_NUM_FOR_ADC) < 0)
    {
        printf("start timer failed!!\n");
    }
    
    /* �ʹ��������һ����������Ҳ����һ����ѭ���� */ 
    for( ;; ) 
    {   
        //����ADC,����DMA
        ADC1_Config();
     
        //����ADC��ʼ������ʵ����adc����timer��������
        start_adc_sample();
        
        //��ȡADC����ֵ
        adc_value = read_adc_average_value(ADC_CHANNEL_NUM);
        //��adc����ֵת��Ϊ��ѹֵ
        voltage = convert_adc_value_to_voltage(adc_value);
        //��adc��ѹֵת��Ϊ�¶�ֵ
        fuser_temperature = convert_voltage_to_temperature(voltage);
        printf("current temperature:%d\n", fuser_temperature);
        //�����¶�ֵ����һ������(���Է�������֪ͨ)
    } 
}


int main(void)
{
    //Hardware init
    //����NVIC�ж����ȼ�����Ϊ��4
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    //Ӳ��ģ���ʼ��
    //ledģ���ʼ��
    //led_init();
    //TIMERģ���ʼ��
    TimerInit(); //��ʱʱ����Ϊ1��
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
