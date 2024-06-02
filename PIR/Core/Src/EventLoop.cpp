#include "EventLoop.h"
#include "main.h"
#include "usbd_cdc_if.h"
#include "gpio.h"
#include "Pir.hpp"

uint8_t buffer[] = "Pir change\r\n";
Pir *myPir = nullptr;

void onPirChangedHandler(Pir* instance)
{
	CDC_Transmit_FS(buffer, sizeof(buffer));
}
void EventLoopCpp() {
	myPir = new Pir(PIR_GPIO_Port, PIR_Pin);
	myPir->attachOnPirChanged(onPirChangedHandler);

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
