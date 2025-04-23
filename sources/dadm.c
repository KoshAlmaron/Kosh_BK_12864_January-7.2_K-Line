// Подключение бибилиотек
#include <stdint.h>			// Коротние название int.
#include <avr/io.h>				// Названия регистров и номера бит.

#include "macros.h"			// Макросы.
#include "pinout.h"			// Начначение выводов контроллера.
#include "bkdata.h"			// Структура с данными.
#include "pinout.h"			// Назначение выводов контроллера.

#include "dadm.h"			// Свой заголовок.

#ifdef INT_OIL_PIN

// Состояние ДАДМ: 0 - ок, 8 - низкое давление, 16 - ошибка.
static uint8_t OilPressureState = 0;
// Флаг, что состояние было отображено на экране.
static uint8_t AlarmShowed = 0;

void dadm_init() {
	SET_PIN_MODE_INPUT(INT_OIL_PIN);
	SET_PIN_LOW(INT_OIL_PIN);

	// Если при старте на линии напряжение, значит обрыв цепи
	// или неисправен датчик.
	if (PIN_READ(INT_OIL_PIN)) {OilPressureState = 16;}
}

void dadm_test() {
	// Выход при обрыве цепи.
	if (OilPressureState == 16) {return;}
	// Сработал датчик - замкнул цепь на массу.
	if (!PIN_READ(INT_OIL_PIN)) {OilPressureState = 8;}
}

uint8_t dadm_get_state(uint16_t RPM) {
	// Сброс состояния датчика на экране текущих ошибок.
	if (BK.ScreenMode == 4 && OilPressureState == 16) {OilPressureState = 0;}
	if (OilPressureState == 16) {return 16;}
	
	if (RPM > 400 && OilPressureState) {
		// Сброс после отображения аварии и нормализации давления.
		if (AlarmShowed) {
			if (BK.AlarmBoxTimer < 0 && OilPressureState == 8 && PIN_READ(INT_OIL_PIN)) {
				OilPressureState = 0;
				AlarmShowed = 0;
			}
		}
		else {
			BK.AlarmBoxTimer = 1;
			AlarmShowed = 1;
		}
		return OilPressureState;
	}
	return 0;
}

#endif
