#ifndef _ADC_H
#define _ADC_H



#define CHANNEL_NUM 4  //转换通道的数�?
#define SAMPLE_NUM 10  //每个通道的转换次�?

typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;



void ADC1_Config(void);
uint16_t read_adc_average_value(uint16_t channel);
void start_adc_sample(void);

uint16_t convert_adc_value_to_voltage(uint16_t adc_value);
uint16_t convert_voltage_to_temperature(uint16_t voltage);


#endif

