#include "EventLoop.h"
#include "main.h"
#include "spi.h"
#include "Tm1638.hpp"
#include "NRF24L.hpp"

GPIO_PinState led_state = GPIO_PIN_SET;
Tm1638 tmDevice = Tm1638(&hspi1, TM_STB_GPIO_Port, TM_STB_Pin,
		TM_MOSI_GPIO_Port, TM_MOSI_Pin);
NRF24L nrfDevice = NRF24L(&hspi2, NRF_CE_GPIO_Port, NRF_CE_Pin, NRF_CSN_GPIO_Port, NRF_CSN_Pin);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == KEY_Pin) {
		led_state = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
	}
}

//#define IS_TX

void EventLoopCpp() {
	tmDevice.test();
	nrfDevice.Init();
	nrfDevice.Check();
	uint8_t payload_length = 5;
	uint8_t nRF24_payload[32];

#ifdef IS_TX
	uint32_t count = 0;
	NRF24L::TXResult tx_res;
	nrfDevice.InitTX();
#else
	nrfDevice.InitRX(nRF24_payload, payload_length);
#endif

	while (1) {
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, led_state);

		uint8_t btns = tmDevice.readButtons();
		for (int i = 0; i < 8; i++) {
			bool val = (btns & (1 << i)) > 0 ? true : false;
			tmDevice.writeLed(i, val);
		}

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
