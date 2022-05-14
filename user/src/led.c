#include "led.h"
#include "stm32f10x_gpio.h"

void led_init(void)
{
	//gpio初始化
    GPIO_InitTypeDef GPIO_InitStruct;
    //配置外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    //配置引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    //使能输出
    GPIO_SetBits(GPIOA, GPIO_Pin_0);
}

//    
////    //关闭内部低速时钟
////    RCC_LSICmd(DISABLE);
////    //关闭外部低速时钟
////    RCC_LSEConfig(RCC_LSE_OFF);
////    //关闭内部高速时钟
////    RCC_HSICmd(DISABLE);
////    //关闭系统休眠时不使用的外设时钟
////    //...
//    
