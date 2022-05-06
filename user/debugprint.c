//把printf输出的数据重定向到ITM端口，使用主机的标准输入/输出设备调试程序

#include <stdio.h>

#define ITM_Port8(n) (*((volatile unsigned char *)(0xE0000000+4*n)))

#define ITM_Port16(n) (*((volatile unsigned short*)(0xE0000000+4*n)))

#define ITM_Port32(n) (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR (*((volatile unsigned long *)(0xE000EDFC)))

#define TRCENA 0x01000000

////关闭mcu半主机模式
//#pragma import(__use_no_semihosting)

//void _sys_exit(int x) 
//{ 
//    x = x; 
//}

//void _ttywrch(int ch)
//{
//    ch = ch;
//}


struct __FILE { int handle; /* Add whatever you need here */ };

    FILE __stdout;

    FILE __stdin;

int fputc(int ch, FILE *f) 
{
    if (DEMCR & TRCENA) 

    {

        while (ITM_Port32(0) == 0);

        ITM_Port8(0) = ch;

    }

    return(ch);
}
