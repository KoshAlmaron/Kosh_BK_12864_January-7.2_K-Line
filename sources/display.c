// Подключение бибилиотек
#include <stdint.h>				// Коротние название int.
#include <avr/interrupt.h>		// Прерывания
#include <avr/wdt.h>			// Сторожевой собак.

#include "oled.h"			// OLED дисплей.
#include "blocks.h"			// Блоки данных для экрана.
#include "bkdata.h"			// Структура с данными.
#include "macros.h"			// Макросы.
#include "configuration.h"	// Настройки.
#include "buttons.h"		// Кнопки.
#include "display.h"		// Свой заголовок.

// Прототипы локальных функций.
static void display_draw_main();
static void display_draw_second();
static void display_draw_acceleration();
static void display_draw_graph();
static void display_draw_screen_change();
static void display_draw_start_stop();

void display_draw_data(uint8_t Timer) {
	if (BK.ScreenChange) {display_draw_screen_change();}
	else if (BK.StartStop) {display_draw_start_stop();}
	else {
		if (BK.ScreenMode != 3) {oled_clear_buffer();}
	}

	if (BK.StartStop == 2) {
		BK.StartStop = 0;
	}
	else if (BK.StartStop == -2) {
		oled_send_buffer();
		return;
	}

	switch (BK.ScreenMode) {
		case 0:		// Основной экран.
			display_draw_main();
			if (BK.ConfigBoxTimer) {draw_config_box(43, 22);}
			break;
		case 1:		// Второй экран.
			display_draw_second();
			break;
		case 2:		// Экран замера разгона.
			display_draw_acceleration();
			break;
		case 3:		// График.
			display_draw_graph();
			break;
		case 4:		// Текущие ошибки.
			draw_current_errors();
			break;
		case 5:		// Сохраненные ошибки.
			draw_saved_errors();
			break;
		case 6:		// АЦП.
			draw_adc_value();
			break;
		case 10:		// Экран завернения.
			draw_statistics();
			break;
	}

	draw_error_box();	// Индикация ошибок.
	oled_send_buffer();

	if (!BK.ScreenChange && !BK.StartStop) {BK.DataStatus = 3;}
}

void display_draw_no_signal() {
	oled_clear_buffer();
	draw_no_signal();
	oled_send_buffer();
}

// Основной экран
static void display_draw_main() {
	// Линии разметки
	oled_draw_h_line(0, 21, 128);
	oled_draw_h_line(0, 43, 128);
	oled_draw_v_line(42, 0, 64);
	oled_draw_v_line(85, 0, 64);

	// ========================== Блоки данных ==========================
	draw_ff_fc_f(0, 0);				// Мгновенный расход топлива (F)
	draw_water_temp_f(0, 22);		// Температура ОЖ (F)
	draw_distance_f(0, 44);			// Суточный и общий пробег (F)

	draw_map_f(43, 0);				// ДАД (F)
	draw_trottle_f(43, 22);			// ДПДЗ / РХХ (F)
	draw_afc_f(43, 44);				// Средний расход топлива суточный и общий (F)

	draw_O2_sensor_h(86, 0);		// AFR (Напряжение УДК) (H)
	draw_inj_corr_h(86, 10);		// Коррекция впрыска (H)
	draw_airtemp_h(86, 22);			// Температура воздуха на впуске (H)
	draw_battery_h(86, 32);			// Напряжение сети (H)
	draw_fuel_burned_f(86, 44);		// Израсходованное топливо (F)

	// ========================== Блоки данных ==========================
}

// Второй экран
static void display_draw_second() {
	// Линии разметки
	oled_draw_h_line(0, 21, 128);
	oled_draw_h_line(0, 43, 128);
	oled_draw_v_line(42, 0, 64);
	oled_draw_v_line(85, 0, 64);

	// ========================== Блоки данных ==========================
	draw_gbc_f(0, 0);
	draw_air_flow_f(0, 22);
	draw_pulse_widgh_f(0, 44);

	draw_rpm_f(43, 0);
	draw_speed_f(43, 22, 1);
	draw_oil_pressure_f(43, 44);

	draw_O2_sensor_h(86, 0);
	draw_inj_corr_h(86, 10);
	draw_airtemp_h(86, 22);
	draw_angle_h(86, 32);
	draw_battery_f(86, 44);
	// ========================== Блоки данных ==========================
}

// Экран замера разгона.
static void display_draw_acceleration() {
	// Линии разметки
	oled_draw_h_line(86, 21, 42);
	oled_draw_h_line(86, 43, 42);
	oled_draw_v_line(85, 0, 64);

	// ========================== Блоки данных ==========================
	draw_speed_f(86, 0, 1);		// Скорость (F)	

	draw_O2_sensor_h(86, 22);	// ДАД (F)	

	draw_water_temp_f(86, 44);	// Температура ОЖ (F)	
	// ========================== Блоки данных ==========================
}

static void display_draw_graph() {
	static uint8_t GraphNumber = 0;	// Номер графика.

	// Смена графика.
	if (buttons_get_state(BTN_UP) == 1 && GraphNumber) {
		GraphNumber--;
		oled_clear_buffer();
	}
	if (buttons_get_state(BTN_DOWN) == 1 && GraphNumber < 8) {
		GraphNumber++;
		oled_clear_buffer();
	}

	oled_draw_h_line(0, 9, 128);
	oled_draw_v_line(102, 0, 64);

	draw_graph(GraphNumber);
}

// Анимация смены экранов.
static void display_draw_screen_change() {
	static uint8_t y = 1;

	switch (BK.ScreenChange) {
		case 1:
			y = 1;
			BK.ScreenChange = 2;
			break;
		case -1:
			y = 1;
			BK.ScreenChange = -2;
			break;
	}

	if (BK.ScreenChange > 0) {
		oled_set_clip_window(0, 0, 127, y);
		oled_draw_box(0, MAX(0, y - CHANGE_ANIMATION_RPC), 128, CHANGE_ANIMATION_RPC * 2, 2);
		oled_draw_h_line(0, y, 128); 
	}
	else {
		oled_set_clip_window(0, 63 - y, 127, 63);
		oled_draw_box(0, MIN(63, 63 - y + CHANGE_ANIMATION_RPC), 128, CHANGE_ANIMATION_RPC * 2, 2);
		oled_draw_h_line(0, 63 - y, 128);
	}
	
	y += CHANGE_ANIMATION_RPC;
	if (y > 63) {
		oled_disable_clip_window();
		oled_clear_buffer();
		BK.ScreenChange = 0;
		return;
	}
}

// Анимация начала/завершения работы
static void display_draw_start_stop() {
	static uint8_t y = 0;

	oled_clear_buffer();

	if (BK.StartStop > 0) {oled_set_clip_window(0, 31 - y, 127, 31 + y);}
	else {oled_set_clip_window(0, y, 127, 63 - y);}

	y += 1;
	if (y > 31) {
		y = 0;
		oled_disable_clip_window();
		oled_clear_buffer();
		BK.StartStop *= 2;
	}

}