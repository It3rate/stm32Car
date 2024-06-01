#include "EventLoop.h"
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usbd_cdc_if.h"

uint8_t buffer[] = "Hello World\r\n";

#define ADC_BUF_LEN 2
uint16_t joyBuf[ADC_BUF_LEN];
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	//CDC_Transmit_FS(buffer, sizeof(buffer));
}
void EventLoopCpp() {
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)joyBuf, ADC_BUF_LEN);
	HAL_TIM_Base_Start(&htim2);
	while (1) {
		//CDC_Transmit_FS(buffer, sizeof(buffer));

	}
}

// Define all C function calls from main.c below
extern "C" {
void EventLoopC() {
	EventLoopCpp();
}
}
