#include <stdint.h>				// Коротние название int.
#include <avr/io.h>				// Названия регистров и номера бит.

#include "configuration.h"		// Настройки.
#include "macros.h"				// Макросы.
#include "pinout.h"				// Начначение выводов контроллера.
#include "spdsens.h"			// Датчик скорости.

#include "speed_chime.h"		// Свой заголовок.

#ifdef SPEED_CHIME_PIN

static uint8_t SpeedChimeStatus = 16;

void speed_chime_init() {
	SET_PIN_MODE_OUTPUT(SPEED_CHIME_PIN);
	SET_PIN_LOW(SPEED_CHIME_PIN);
}

void speed_chime_control(uint8_t TimeAdd) {
	static uint16_t Timer = 0;
	
	Timer += TimeAdd;
	uint8_t Speed = get_car_speed();
	uint16_t SpeedChimeInterval = MAX(500, SPEED_CHIME_INTERVAL - ABS(SPEED_CHIME_LIMIT - Speed) * 10);
	switch (SpeedChimeStatus) {
		case 0:		// Включение.
			if (Speed > SPEED_CHIME_LIMIT) {
				Timer = 0;
				SpeedChimeStatus = 1;
				SET_PIN_HIGH(SPEED_CHIME_PIN);
			}
			else {Timer = 0;}
			break;
		case 1:		// Выключение.
			if (Timer >= SPEED_CHIME_DELAY) {
				SpeedChimeStatus = 2;
				Timer = 0;
				SET_PIN_LOW(SPEED_CHIME_PIN);
			}
			break;
		case 2:		// Пауза перед новым циклом.
			if (Timer >= SpeedChimeInterval) {
				if (Speed > SPEED_CHIME_LIMIT) {
					Timer = 0;
					SpeedChimeStatus = 1;
					SET_PIN_HIGH(SPEED_CHIME_PIN);
				}
				else {
					Timer = 0;
					SpeedChimeStatus = 0;
				}
			}
			break;
		case 16:	// Пауза после запуска БК.
			if (Timer >= 3000) {
				Timer = 0;
				SpeedChimeStatus = 0;
			}
			break;
	}
}

#endif