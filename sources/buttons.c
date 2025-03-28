#include <stdint.h>				// Коротние название int.
#include <avr/io.h>				// Названия регистров и номера бит.

#include "configuration.h"		// Настройки.
#include "macros.h"				// Макросы.
#include "pinout.h"				// Начначение выводов контроллера.
#include "bkdata.h"				// Структура с данными.
#include "uart.h"				// UART.
#include "oled.h"				// OLED дисплей.
#include "adc.h"				// АЦП.

#include "buttons.h"			// Свой заголовок.

/*
	Состояние кнопок:
		0 - не нажата,
		1..99 - ожидание после нажатия,
		100 - событие обработано,
		101..149 - откат после срабатывания,
		201 - короткое нажатие,
		202 - длинное нажатие.
*/
static uint8_t ButtonState[4] = {};	//Ввер/внизх, влево/вправо.

// Прототипы функций.
static void button_read(uint8_t State, uint8_t N);

void buttons_init() {
	// Настраиваем выводы для кнопок/энкодера.
	SET_PIN_MODE_INPUT(BUTTON_UP_PIN);
	SET_PIN_HIGH(BUTTON_UP_PIN);
	SET_PIN_MODE_INPUT(BUTTON_DOWN_PIN);
	SET_PIN_HIGH(BUTTON_DOWN_PIN);
	SET_PIN_MODE_INPUT(BUTTON_RIGHT_PIN);
	SET_PIN_HIGH(BUTTON_RIGHT_PIN);
	#ifndef ENCODER_CONTROL
		SET_PIN_MODE_INPUT(BUTTON_LEFT_PIN);
		SET_PIN_HIGH(BUTTON_LEFT_PIN);
	#endif
}

void button_action() {
	// Влево/вправо короткое смена экрана
	if (buttons_get_state(BTN_LEFT) == 1) {BK.ScreenChange = -1;
		if (!BK.ScreenMode) {BK.ScreenMode = LCD_LAST_DISPLAY;}
		else {BK.ScreenMode -= 1;}
	}
	if (buttons_get_state(BTN_RIGHT) == 1) {
		BK.ScreenChange = 1;
		if (BK.ScreenMode == LCD_LAST_DISPLAY) {BK.ScreenMode = 0;}
		else {BK.ScreenMode += 1;}
	}

	int8_t BrightAdd = 0;
	switch (BK.ScreenMode) {
		case 0:		// Основной экран.
			// ==================== ЯРКОСТЬ ЭКРАНА ====================
			if (buttons_get_state(BTN_UP) == 1) {BrightAdd = LCD_BRIGHT_STEP;}			// Коротое вверх.
			if (buttons_get_state(BTN_DOWN) == 1) {BrightAdd = -1 * LCD_BRIGHT_STEP;}	// Коротое вниз.

			if (BrightAdd) {
				if (!BK.ConfigBoxTimer) {BK.ConfigBoxTimer = BRIGHT_BOX_TIMER;}
				else {
					BK.ConfigBoxTimer = BRIGHT_BOX_TIMER;
					#ifdef AUTO_BRIGHT_PIN
						if (get_adc_value() < 512) {
							BK.BrightLCD[1] = CONSTRAIN(BK.BrightLCD[1] + BrightAdd, 0, 255);
						}
						else {
							BK.BrightLCD[2] = CONSTRAIN(BK.BrightLCD[2] + BrightAdd, 0, 255);	
						}
					#else
						BK.BrightLCD[0] = CONSTRAIN(BK.BrightLCD[0] + BrightAdd, LCD_BRIGHT_MIN, LCD_BRIGHT_MAX);
						oled_set_bright(BK.BrightLCD[0]);
					#endif
				}
			}
			// ==================== ЯРКОСТЬ ЭКРАНА ====================
		
			if (buttons_get_state(BTN_UP) == 2) { 	// Длинное вверх.
				// Сброс cуточного пробега и пробега за поездку.
				BK.DistRide = 0;
				BK.FuelRide = 0;
				BK.DistDay = 0;
				BK.FuelDay = 0;
				BK.RideTimer = 0;
			}
			break;			
		case 4:		// Сохраненные ошибки.
			if (buttons_get_state(BTN_UP) == 2) { 	// Длинное вверх.
				uart_send_command(COMMAND_CDI);	// Сброс ошибок CE.
				BK.DataStatus = 16;
			}
			break;
	}
}

void buttons_clear() {
	if (PIN_READ(BUTTON_UP_PIN) && ButtonState[BTN_UP] > 200) {ButtonState[BTN_UP] = 100;}
	if (PIN_READ(BUTTON_DOWN_PIN) && ButtonState[BTN_DOWN] > 200) {ButtonState[BTN_DOWN] = 100;}
	if (PIN_READ(BUTTON_LEFT_PIN) && ButtonState[BTN_LEFT] > 200) {ButtonState[BTN_LEFT] = 100;}
	if (PIN_READ(BUTTON_RIGHT_PIN) && ButtonState[BTN_RIGHT] > 200) {ButtonState[BTN_RIGHT] = 100;}
}

// Вызов каждые 25 мс.
void buttons_update() {
	button_read(BTN_UP, PIN_READ(BUTTON_UP_PIN));
	button_read(BTN_DOWN, PIN_READ(BUTTON_DOWN_PIN));
	button_read(BTN_LEFT, PIN_READ(BUTTON_LEFT_PIN));
	button_read(BTN_RIGHT, PIN_READ(BUTTON_RIGHT_PIN));
}

uint8_t buttons_get_state(uint8_t N) {
	switch (ButtonState[N]) {
		case 201:	// Короткое нажатие.
			return 1;
			break;
		case 202:	// Длинное нажатие.	
			return 2;
			break;
	}
	return 0;
}

static void button_read(uint8_t N, uint8_t State) {
	if (ButtonState[N] < 100) {
		if (!State) {
			ButtonState[N]++;
			if (ButtonState[N] >= 60) {ButtonState[N] = 202;}	// Длиное нажатие.
		}
		else {
			if (ButtonState[N] >= 2) {ButtonState[N] = 201;}	// Короткое нажатие.
		}
		return;
	}

	if (ButtonState[N] < 150 && State) {
		ButtonState[N]++;
		if (ButtonState[N] >= 105) {
			ButtonState[N] = 0;		// Сброс состояния кнопки.
		}
		return;		
	}
}


