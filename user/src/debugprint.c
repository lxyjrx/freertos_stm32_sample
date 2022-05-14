//把printf输出的数据重定向到ITM端口，使用主机的标准输入/输出设备调试程序

#include"stm32f10x.h"

#define AC6_ENABLE
 	  
#ifdef AC6_ENABLE
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	
__ASM (".global __use_no_semihosting");      

#else                                                       //AC5_ENABLE
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)	
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

#endif

typedef struct __FILE 
{ int handle; /* Add whatever you need here */ 
}FILE;

FILE __stdout;

FILE __stdin;

void _sys_exit(int return_code)  
{  
label:  
    goto label;  /* endless loop */  
}  

int fputc(int ch, FILE *f)   
{  
        return ITM_SendChar(ch);  
}  

volatile int32_t ITM_RxBuffer;  
int fgetc(FILE *f)  
{  
      while (ITM_CheckChar() != 1) __NOP();  
      return (ITM_ReceiveChar());  
}  


int __backspace()  
{  
    return 0;  
}  

void _ttywrch(int c)  
{  
    fputc(c, 0);  
} 



  
/*************   printf重定向到ITM端口（旧的实现）****************************/

//#define ITM_Port8(n) (*((volatile unsigned char *)(0xE0000000+4*n)))

//#define ITM_Port16(n) (*((volatile unsigned short*)(0xE0000000+4*n)))

//#define ITM_Port32(n) (*((volatile unsigned long *)(0xE0000000+4*n)))

//#define DEMCR (*((volatile unsigned long *)(0xE000EDFC)))

//#define TRCENA 0x01000000

//////关闭mcu半主机模式
////#pragma import(__use_no_semihosting)

////void _sys_exit(int x) 
////{ 
////    x = x; 
////}

////void _ttywrch(int ch)
////{
////    ch = ch;
////}


//struct __FILE { int handle; /* Add whatever you need here */ };

//    FILE __stdout;

//    FILE __stdin;



//int fputc(int ch, FILE *f) 
//{
//    if (DEMCR & TRCENA)     //使用ITM port0
//    {
//        while (ITM_Port32(0) == 0);

//        ITM_Port8(0) = ch;
//    }

//    return(ch);
//}
