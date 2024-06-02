
#include "Pir.hpp"

static Pir* _instance = nullptr;

Pir::Pir(GPIO_TypeDef *port, uint16_t pin) :
	_port(port), _pin(pin), _state(false)
{
	_instance = this;
}

Pir::~Pir() {
}

bool Pir::GetState()
{
	return _state;
}

void Pir::attachOnPirChanged(PirChangedHandler handler)
{
    onPirChanged = handler;
}

void Pir::OnPirChange(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == _pin)
	{
		_state = !_state;
	}

	if (onPirChanged != nullptr)
	{
        onPirChanged(this);
    }
}

extern void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (_instance != nullptr) {
        _instance->OnPirChange(GPIO_Pin);
    }
}
