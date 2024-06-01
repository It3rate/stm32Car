#include "EventLoop.h"
#include "main.h"
#include "spi.h"
#include "adc.h"
#include "usart.h"
#include "Tm1638.hpp"
#include "NRF24L.hpp"/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

GPIO_PinState led_state = GPIO_PIN_SET;

//uint8_t _uart_tx[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x08, 0x00, 0x12, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x04, 0x03, 0x02, 0x01};
uint8_t _uart_tx[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x00, 0x00, 0x04, 0x03, 0x02, 0x01};

uint8_t RX_BUF[64]={0};  //缓存数组
uint8_t RX_count=0;      //计数位
uint8_t RX_temp;         //缓存字符

Tm1638 tmDevice = Tm1638(&hspi1, TM_STB_GPIO_Port, TM_STB_Pin, TM_MOSI_GPIO_Port, TM_MOSI_Pin);

NRF24L nrfDevice = NRF24L(&hspi2, NRF_CE_GPIO_Port, NRF_CE_Pin, NRF_CSN_GPIO_Port, NRF_CSN_Pin);

uint8_t buffer[] = "Hello World\n";

#define ADC_BUF_LEN 6
uint32_t _adcBuf[ADC_BUF_LEN];

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == KEY_Pin) {
		led_state = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	CDC_Transmit_FS(RX_BUF, sizeof(RX_BUF));
}

void inter_cmd(void)
{
    uint8_t cmd[]={0xfd,0xfc,0xfb,0xfa,0x04,0x00,0xff,0x00,0x01,0x00,0x04,0x03,0x02,0x01};
    HAL_UART_Transmit(&huart1,(uint8_t *)&cmd,sizeof(cmd),0xFFFF);
    HAL_Delay(150);
    HAL_UART_Transmit(&huart1,(uint8_t *)&cmd,sizeof(cmd),0xFFFF);
}

void close_cmd(void)
{
    uint8_t close_cmd[]={0xfd,0xfc,0xfb,0xfa,0x02,0x00,0xfe,0x00,0x04,0x03,0x02,0x01};
    HAL_UART_Transmit(&huart1,(uint8_t *)&close_cmd,sizeof(close_cmd),0xFFFF);
}

void RD_03_Write_cmd(uint8_t parameter,uint8_t data)
{
    inter_cmd();
    uint8_t Write_cmd[]={0xfd,0xfc,0xfb,0xfa,0x08,0x00,0x07,0x00,parameter,0x00,data,0x00,0x00,0x00,0x04,0x03,0x02,0x01};
    HAL_UART_Transmit(&huart1,(uint8_t *)&Write_cmd,sizeof(Write_cmd),0xFFFF);
    close_cmd();
}

//#define IS_TX

void EventLoopCpp() {
	tmDevice.test();
	nrfDevice.Init();
	nrfDevice.Check();
	uint8_t payload_length = 5;
	uint8_t nRF24_payload[32];

	//HAL_ADC_Start_DMA(&hadc1, (uint32_t*)_adcBuf, ADC_BUF_LEN);
	RD_03_Write_cmd(1,2);
	HAL_UART_Receive_IT(&huart1,&RX_temp,1);

//    HAL_Delay(1000);
//    uint8_t cmd[]={0xfd,0xfc,0xfb,0xfa,0x04,0x00,0xff,0x00,0x01,0x00,0x04,0x03,0x02,0x01};
//    HAL_UART_Transmit(&huart1,(uint8_t *)&cmd,sizeof(cmd),0xFFFF);
//    HAL_Delay(150);
//    HAL_UART_Transmit(&huart1,(uint8_t *)&cmd,sizeof(cmd),0xFFFF);

#ifdef IS_TX
	uint32_t count = 0;
	NRF24L::TXResult tx_res;
	nrfDevice.InitTX();
#else
	nrfDevice.InitRX(nRF24_payload, payload_length);
#endif

	while (1) {
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, led_state);
	CDC_Transmit_FS(buffer, sizeof(buffer));
		HAL_Delay(100);

		uint8_t btns = tmDevice.readButtons();
		for (int i = 0; i < 8; i++) {
			bool val = (btns & (1 << i)) > 0 ? true : false;
			tmDevice.writeLed(i, val);
		}

//		HAL_UART_Transmit(&huart1, _uart_tx, sizeof(_uart_tx), 100);
//		HAL_Delay(100);
//		HAL_UART_Receive(&huart1, _uart_rx, sizeof(_uart_rx), 100);
//		HAL_Delay(100);

#ifdef IS_TX
	nRF24_payload[0] = count & 0xFF;
	nRF24_payload[1] = (count >> 8) & 0xFF;
	nRF24_payload[2] = (count >> 16) & 0xFF;
	nRF24_payload[3] = (count >> 24) & 0xFF;
	nRF24_payload[4] = 0xAA;
	// Transmit a packet
	tx_res = nrfDevice.TransmitPacket(nRF24_payload, payload_length);
	switch (tx_res) {
		case NRF24L::TX_SUCCESS:
			break;
		case NRF24L::TX_TIMEOUT:
			break;
		case NRF24L::TX_MAXRT:
			break;
		default:
			break;
	}
	count++;
#else // RX
		uint8_t status = nrfDevice.GetStatus_RXFIFO();
		if (status != NRF24L::FifoStatus::EMPTY) {
			nrfDevice.ReadPayload(nRF24_payload, &payload_length);
			nrfDevice.ClearIRQFlags();

			tmDevice.writeHexTo(6, 2, nRF24_payload[0]);
			tmDevice.writeHexTo(4, 2, nRF24_payload[1]);
			tmDevice.writeHexTo(2, 2, nRF24_payload[2]);
			tmDevice.writeHexTo(0, 2, nRF24_payload[3]);
		}
#endif
	}
}

// Define all C function calls from main.c below
extern "C" {
void EventLoopC() {
	EventLoopCpp();
}
}
