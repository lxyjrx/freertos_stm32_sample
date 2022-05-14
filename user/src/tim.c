#include "tim.h"
#include <stdio.h>

#include"stm32f10x.h"

volatile uint16_t s_tim5_exceed = 0;   //定时器计数溢出次数
volatile uint8_t s_tim5_IC_edge = 0;  //捕获边沿计数
volatile uint16_t s_tim5_IC_val = 0;   //存放计数器CNT的值

volatile uint16_t s_tim3_exceed = 0;   //定时器计数溢出次数
volatile uint8_t s_tim3_IC_edge = 0;  //捕获边沿计数
volatile uint16_t s_tim3_IC_val = 0;   //存放计数器CNT的值
volatile uint16_t s_tim3_pwminput_dutycycle = 0;
volatile uint16_t s_tim3_pwminput_freq = 0;


uint16_t s_timer2_duty = 0;

#define TIM3_ARR    2000
#define TIM3_TIME_BASE  (72000000/36000)
#define SECONDS_TO_MICROSECONDS 1000000

#define PWM_INPUT_MODE  1
#define INPUT_CAP_MODE  0

static void TIME3_Init(uint16_t arr,uint16_t psc)//arr为自动装载值，psc为预分频系数
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitTypeStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;
	NVIC_InitTypeDef NVIC_InitTypeStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能GPIOA时钟


    //PA6引脚GPIO初始化,用于捕获输入的pwm信号,测量其周期
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
	//定时器初始化
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_TimeBaseInitTypeStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitTypeStruct.TIM_CounterMode = TIM_CounterMode_Up;//计数方式
	TIM_TimeBaseInitTypeStruct.TIM_Period = arr;//自动装载值
	TIM_TimeBaseInitTypeStruct.TIM_Prescaler = psc;//预分频系数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitTypeStruct);
    
    //清除定时器更新标志
    TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
    
    //使能更新事件触发trigger output,触发ADC采样
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);

#if INPUT_CAP_MODE
    //channel1 设置捕获参数
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStruct.TIM_ICFilter = 0xf;    //采样频率设为0.56MHz,连续采8次，可以滤掉14微秒的干扰
    TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;  //捕获极性
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1; //每检测到一个有效边沿就捕获一次
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInit(TIM3, &TIM_ICInitStruct);

#elif PWM_INPUT_MODE
    //channel1 设置捕获参数
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStruct.TIM_ICFilter = 0xf;    //采样频率设为0.56MHz,连续采8次，可以滤掉14微秒的干扰
    TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;  //捕获极性
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1; //每检测到一个有效边沿就捕获一次
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI; 
    TIM_PWMIConfig(TIM3, &TIM_ICInitStruct);
    TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Disable);
#endif

	//使能定时器的中断,  包括更新中断、CC1捕获中断
	TIM_ITConfig(TIM3,TIM_IT_Update|TIM_IT_CC1,ENABLE);
	
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
    GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitTypeStruct;
	
    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能GPIOA、复用功能时钟
	
    //PA1引脚GPIO初始化,用于输出pwm
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    //PA2引脚GPIO初始化，用于输出pwm
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
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
    
    //channel2 初始化输出比较参数
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;  //输出极性高电平有效
    TIM_OCInitStruct.TIM_Pulse = s_timer2_duty; //计时500ms后翻转一次
    TIM_OC2Init(TIM2, &TIM_OCInitStruct);

    //channel3 初始化输出比较参数
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;  //输出极性高电平有效
    TIM_OCInitStruct.TIM_Pulse = s_timer2_duty; //计时500ms后翻转一次
    TIM_OC3Init(TIM2, &TIM_OCInitStruct);
    
    //TIM_ARRPreloadConfig(TIM2, ENABLE);
    //TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    //TIM_CtrlPWMOutputs(TIM2, ENABLE);
    

    
//    //使能更新事件触发trigger output
//	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
//	//使能定时器的中断
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC2|TIM_IT_CC3,ENABLE);
	
	//使能NVIC irq通道
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

static void TIME5_Init(uint16_t arr,uint16_t psc)//arr为自动装载值，psc为预分频系数
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;
	NVIC_InitTypeDef NVIC_InitTypeStruct;
	GPIO_InitTypeDef GPIO_InitStruct;

    TIM_DeInit(TIM5);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);//时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能GPIOA、复用功能时钟
	
    //PA1引脚GPIO初始化,用于捕获pwm信号
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
        
	//定时器初始化，时间基准是500微秒
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;    //设置数字滤波器的时钟分频
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//向上计数方式
	TIM_TimeBaseInitStruct.TIM_Period = arr;//自动装载值
	TIM_TimeBaseInitStruct.TIM_Prescaler = psc;//预分频系数
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStruct);
    
    //清除定时器更新标志
    //TIM_ClearITPendingBit(TIM5,TIM_IT_Update);
    
    //channel1 设置捕获参数
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStruct.TIM_ICFilter = 0xf;    //采样频率设为0.56MHz,连续采8次，可以滤掉14微秒的干扰
    TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;  //捕获极性
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1; //每检测到一个有效边沿就捕获一次
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInit(TIM5, &TIM_ICInitStruct);

    //TIM_ARRPreloadConfig(TIM5, ENABLE);
    
	//使能定时器的中断
	TIM_ITConfig(TIM5,TIM_IT_Update|TIM_IT_CC1,ENABLE);
	
	//使能MVIC irq通道
	NVIC_InitTypeStruct.NVIC_IRQChannel = TIM5_IRQn;//中断通道 
	NVIC_InitTypeStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitTypeStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitTypeStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitTypeStruct);
	
	//使能定时器
	TIM_Cmd(TIM5,ENABLE);
}

void TIM2_IRQHandler(void)
{
    //判断是否为定时器2产生的更新中断
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
        
        s_timer2_duty = s_timer2_duty<<1;
        if(s_timer2_duty >= 32767)
        {
            s_timer2_duty = 20;
        }
		TIM_SetCompare2(TIM2, s_timer2_duty);    //pwm占空比设置为50%
		TIM_SetCompare3(TIM2, s_timer2_duty);    //pwm占空比设置为50%
        TIM_SetAutoreload(TIM2, (s_timer2_duty<<1));
		//要手动的清理中断标志位
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
    
    if(TIM_GetITStatus(TIM2,TIM_IT_CC2)==SET)   //比较中断，当CC2通道检测到计数值等于比较值就触发中断
	{
		
		//要手动的清理中断标志位
		TIM_ClearITPendingBit(TIM2,TIM_IT_CC2);
	}
    if(TIM_GetITStatus(TIM2,TIM_IT_CC3)==SET)   //比较中断，当CC2通道检测到计数值等于比较值就触发中断
	{
		
		//要手动的清理中断标志位
		TIM_ClearITPendingBit(TIM2,TIM_IT_CC3);
	}
}

//中断服务函数
void TIM3_IRQHandler(void)
{
	//判断是否为定时器3产生的中断
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
	{   
        if(s_tim3_IC_edge == 1)		//一个pwm周期内计数器溢出计数
		{
			s_tim3_exceed++;
		}
        
		//要手动的清理中断标志位
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}

#if INPUT_CAP_MODE
    if(TIM_GetITStatus(TIM3,TIM_IT_CC1)==SET)   //捕获中断
    {
        TIM_Cmd(TIM3, DISABLE);    //定时器5失能
        //捕获到上边沿，计数值加一
        s_tim3_IC_edge++;
        if(s_tim3_IC_edge == 1)
        {
            TIM_Cmd(TIM3, DISABLE);     //timer5失能
            TIM_SetCounter(TIM3, 0);    //清除计数器
            
            s_tim3_exceed = 0;             //清除定时器溢出次数
            TIM_Cmd(TIM3, ENABLE);      //timer5使能
        }
        else
        {
            TIM_Cmd(TIM3, DISABLE); //timer5失能
            
            s_tim3_IC_val = TIM_GetCapture1(TIM3);
            s_tim3_IC_edge = 0;
        }

        //要手动的清理中断标志位
        TIM_ClearITPendingBit(TIM3,TIM_IT_CC1);
    }
    
#elif PWM_INPUT_MODE
    if(TIM_GetITStatus(TIM3,TIM_IT_CC1)==SET)   //捕获CC1中断
    {
        //要手动的清理中断标志位
        TIM_ClearITPendingBit(TIM3,TIM_IT_CC1);
        
        /* Get the Input Capture value */
        s_tim3_IC_val = TIM_GetCapture1(TIM3);

        s_tim3_IC_edge++;
        if (s_tim3_IC_edge > 1)
        {
            /* Duty cycle computation */
            s_tim3_pwminput_dutycycle = (TIM_GetCapture2(TIM3) * 100) / s_tim3_IC_val;

            /* Frequency computation */
            s_tim3_pwminput_freq = TIM3_TIME_BASE / s_tim3_IC_val;
        }
        else
        {
            s_tim3_pwminput_dutycycle = 0;
            s_tim3_pwminput_freq = 0;
        }
    }
#endif
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

void TIM5_IRQHandler(void)
{
	//判断是否为更新中断
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)==SET)
	{
		//要手动的清理中断标志位
		TIM_ClearITPendingBit(TIM5,TIM_IT_Update);
        
	}
    
    if(TIM_GetITStatus(TIM5,TIM_IT_CC1)==SET)   //捕获中断
    {
        TIM_Cmd(TIM5, DISABLE);    //定时器5失能
        //捕获到上边沿，计数值加一
        s_tim5_IC_edge++;
        if(s_tim5_IC_edge == 1)
        {
            TIM_Cmd(TIM5, DISABLE);     //timer5失能
            TIM_SetCounter(TIM5, 0);    //清除计数器
            
            s_tim5_exceed = 0;             //清除定时器溢出次数
            TIM_Cmd(TIM5, ENABLE);      //timer5使能
        }
        else
        {
            TIM_Cmd(TIM5, DISABLE); //timer5失能
            
            s_tim5_IC_val = TIM_GetCapture1(TIM5);
            s_tim5_IC_edge = 0;
        }

        //要手动的清理中断标志位
        TIM_ClearITPendingBit(TIM5,TIM_IT_CC1);
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

uint16_t timer_get_pwm_period(void)
{
    uint16_t ret;
    
    
#if INPUT_CAP_MODE
    ret = (s_tim3_IC_val + s_tim3_exceed*TIM3_ARR)*(SECONDS_TO_MICROSECONDS/TIM3_TIME_BASE);
    ret = ret/1000; //微秒转化为毫秒
#else
    if(s_tim3_IC_edge > 1)
    {
        s_tim3_IC_edge = 0;
    }
    ret = s_tim3_pwminput_freq;
#endif
    return ret;
}

uint16_t timer_get_pwm_dutycycle(void)
{
    return s_tim3_pwminput_dutycycle;
}
//==============================END=====================================//


void TimerInit(void)
{
    //设置一个定时器，定时触发adc采样/输入捕获pwm,测量其频率和占空比
    TIME3_Init(20000,3600); //时基是50微秒
    
    //设置一个定时器，输出pwm信号
    s_timer2_duty = 1000;
    TIME2_Init((s_timer2_duty<<1),36000);   
    
    //初始化一个定时器，用来统计系统运行时间(时基为500微秒，最多监视32.768秒)
    //TIME4_Init(65535,3600);

    //初始化一个定时器，实现pwm信号输入捕获
    //TIME5_Init(2000, 36000);
}

