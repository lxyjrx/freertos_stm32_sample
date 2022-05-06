#include "tim.h"
#include <stdio.h>

#include"stm32f10x.h"

uint16_t s_timer2_duty = 0;

static void TIME3_Init(uint16_t arr,uint16_t psc)//arr为自动装载值，psc为预分频系数
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitTypeStruct;
	NVIC_InitTypeDef NVIC_InitTypeStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//时钟使能
	
	//定时器初始化
	TIM_TimeBaseInitTypeStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitTypeStruct.TIM_CounterMode = TIM_CounterMode_Up;//计数方式
	TIM_TimeBaseInitTypeStruct.TIM_Period = arr;//自动装载值
	TIM_TimeBaseInitTypeStruct.TIM_Prescaler = psc;//预分频系数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitTypeStruct);
    
    //清除定时器更新标志
    TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
    
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    //使能更新事件触发trigger output
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
	//使能定时器的中断
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
	//中断优先级分组
	NVIC_InitTypeStruct.NVIC_IRQChannel = TIM3_IRQn;//中断通道 
	NVIC_InitTypeStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitTypeStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitTypeStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitTypeStruct);
	
	//使能定时器
	TIM_Cmd(TIM3,ENABLE);
}

static void TIME2_Init(uint16_t arr,uint16_t psc)//arr为自动装载值，psc为预分频系数
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitTypeStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
	NVIC_InitTypeDef NVIC_InitTypeStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能GPIOA、复用功能时钟
	
    //PA1引脚GPIO初始化
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
	//定时器初始化，时间基准是500微秒
	TIM_TimeBaseInitTypeStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitTypeStruct.TIM_CounterMode = TIM_CounterMode_Up;//计数方式
	TIM_TimeBaseInitTypeStruct.TIM_Period = arr;//自动装载值
	TIM_TimeBaseInitTypeStruct.TIM_Prescaler = psc;//预分频系数
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitTypeStruct);
    
    //清除定时器更新标志
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
    
     //初始化输出比较参数
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;  //输出极性高电平有效
    TIM_OCInitStruct.TIM_Pulse = s_timer2_duty; //计时500ms后翻转一次
    TIM_OC2Init(TIM2, &TIM_OCInitStruct);
    
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    //TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    //TIM_CtrlPWMOutputs(TIM2, ENABLE);
    
//    //使能更新事件触发trigger output
//	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
//	//使能定时器的中断
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC2,ENABLE);
	
	//中断优先级分组
	NVIC_InitTypeStruct.NVIC_IRQChannel = TIM2_IRQn;//中断通道 
	NVIC_InitTypeStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitTypeStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitTypeStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitTypeStruct);
	
	//使能定时器
	TIM_Cmd(TIM2,ENABLE);
}

void TIME4_Init(uint16_t arr,uint16_t psc)//arr为自动装载值，psc为预分频系数
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitTypeStruct;
	NVIC_InitTypeDef NVIC_InitTypeStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);//时钟使能
	
	//定时器初始化
	TIM_TimeBaseInitTypeStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitTypeStruct.TIM_CounterMode = TIM_CounterMode_Up;//计数方式
	TIM_TimeBaseInitTypeStruct.TIM_Period = arr;//自动装载值
	TIM_TimeBaseInitTypeStruct.TIM_Prescaler = psc;//预分频系数
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitTypeStruct);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    
	//使能定时器的中断
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
	
	//中断优先级分组
	NVIC_InitTypeStruct.NVIC_IRQChannel = TIM4_IRQn;//中断通道 
	NVIC_InitTypeStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitTypeStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitTypeStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitTypeStruct);

	//使能定时器
	TIM_Cmd(TIM4,ENABLE);
}

//中断服务函数
void TIM3_IRQHandler(void)
{
	//判断是否为定时器3产生的中断
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
	{   
		//要手动的清理中断标志位
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}
}

void TIM2_IRQHandler(void)
{
    //判断是否为定时器2产生的更新中断
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
        
        s_timer2_duty = s_timer2_duty<<1;
        if(s_timer2_duty >= 65500)
        {
            s_timer2_duty = 20;
        }
		TIM_SetCompare2(TIM2, s_timer2_duty);    //pwm占空比设置为50%
        TIM_SetAutoreload(TIM2, (s_timer2_duty<<1));
		//要手动的清理中断标志位
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
    
    if(TIM_GetITStatus(TIM2,TIM_IT_CC2)==SET)   //比较捕获中断，当CC2通道检测到计数值等于比较值就触发中断
	{
		
		//要手动的清理中断标志位
		TIM_ClearITPendingBit(TIM2,TIM_IT_CC2);
	}
}

void TIM4_IRQHandler(void)
{
	//判断是否为定时器3产生的中断
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET)
	{
		//要手动的清理中断标志位
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	}
}

//==========================TIMER模块对外提供的接口=======================//
int TimerStart(uint8_t timer_to_start)
{
    if(timer_to_start < 2 || timer_to_start > 4)
    {
        printf("no this timer,start failed!!!\n");
        return -1;
    }

    switch (timer_to_start)
    {
        case 2:
            TIM_Cmd(TIM2,ENABLE);
            break;
        case 3:
            TIM_Cmd(TIM3,ENABLE);
            break;
        case 4:
            TIM_Cmd(TIM4,ENABLE);
            break;
        default:
            break;
    }

    return 0;
}

int TimerStop(uint8_t timer_to_stop)
{
    if(timer_to_stop < 2 || timer_to_stop > 4)
    {
        printf("no this timer,start failed!!!\n");
        return -1;
    }

    switch (timer_to_stop)
    {
        case 2:
            TIM_Cmd(TIM2,DISABLE);
            break;
        case 3:
            TIM_Cmd(TIM3,DISABLE);
            break;
        case 4:
            TIM_Cmd(TIM4,DISABLE);
            break;
        default:
            break;
    }

    return 0;
}


//==============================END=====================================//
void TimerInit(void)
{
    //设置一个定时器，定时触发adc采样
    TIME3_Init(2000,36000);
    
    //设置一个定时器，输出pwm信号
    s_timer2_duty = 20;
    TIME2_Init((s_timer2_duty<<1),36000);
    
    //初始化一个定时器，用来统计系统运行时间(时基为500微秒，最多监视32.768秒)
    //TIME4_Init(65535,3600);
}

