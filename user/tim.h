#ifndef TIM_H
#define TIM_H


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

void TimerInit(void);
int TimerStart(uint8_t timer_to_start);
int TimerStop(uint8_t timer_to_stop);



#endif
