#include "uart.h"


#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"


static void UART_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开时钟

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;     //UART TX 推挽复用输出
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;  
    GPIO_Init(GPIOA,&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //UART RX 浮空输入
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
}

static void UART_Config(void)
{
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
    //配置波特率
    USART_InitStructure.USART_BaudRate = BaudRate;
    //硬件流控
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    //配置收发同时进行模式
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx ;
    //没有校验位
    USART_InitStructure.USART_Parity = USART_Parity_No;
    //一位停止位
    USART_InitStructure.USART_StopBits = USART_StopBits_1 ;
    //配置数据字长
    USART_InitStructure.USART_WordLength = USART_WordLength_8b ;
    USART_Init(USART1,&USART_InitStructure);

    USART_Cmd(USART1,ENABLE);
}

void UART_Init(void)
{
    UART_GPIO_Config();
    UART_Config();
}

////__ASM (".global __use_no_semihosting");  
//typedef struct __FILE 
//{ int handle; /* Add whatever you need here */ 
//}FILE;




//int fputc(int ch, FILE * f)
//{
//    //等待串口数据发送完毕
//    while((USART1->SR & USART_FLAG_TC) == 0);
//    
//    //发送下一个字符
//    USART1->DR = (uint8_t)ch;

//    return ch;
//}


