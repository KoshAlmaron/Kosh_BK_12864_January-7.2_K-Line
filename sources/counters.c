#include <avr/interrupt.h>		// Прерывания.
#include <stdint.h>				// Коротние название int.
#include <avr/io.h>				// Названия регистров и номера бит.

#include "bkdata.h"				// Структура с данными.
#include "macros.h"				// Макросы.
#include "configuration.h"		// Настройки.
#include "counters.h"			// Свой заголовок.

#define INT_0_PIN D, 2
#define INT_1_PIN D, 3

void counters_init() {
	// Настройка входов с подтяжкой.
	SET_PIN_MODE_INPUT(INT_0_PIN);
	SET_PIN_HIGH(INT_0_PIN);

	EICRA |= (1 << ISC01);		// INT0 Спадающий фронт.
	EIMSK |= (1 << INT0);		// INT0 Включен.
}


// Прерывание на импульсы расхода топлива.
ISR (INT0_vect) {
	static uint8_t FCounter = 0;

	FCounter++;
	if (FCounter >= IMPULSES_PER_MLITER) {
		BK.FuelRide++;
		FCounter = 0;
	}

}
