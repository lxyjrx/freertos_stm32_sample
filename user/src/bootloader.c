//bootloader文件，用作IAP程序(在线应用编程)
#include <stdio.h>
#include "bootloader.h"


void bootloader_main( void )__attribute((section(".text")));
void bootloader_main( void )
{ 
    printf("system start ...\n");
    
    
    
}
