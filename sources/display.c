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

// Основной экран
static void display_draw_main() {
	// Линии разметки
	oled_draw_h_line(0, 21, 128);
	oled_draw_h_line(0, 43, 128);
	oled_draw_v_line(42, 0, 64);
	oled_draw_v_line(85, 0, 64);

	// ========================== Блоки данных ==========================
    draw_ff_fc_f(0, 0);             // Мгновенный расход топлива (F)	
    draw_water_temp_f(0, 22);       // Температура ОЖ (F)	
    draw_distance_f(0, 44);         // Суточный и общий пробег (F)	
    draw_map_f(43, 0);              // ДАД (F)	
    draw_speed_f(43, 22, 0);        // Скорость с ЭБУ (F)	
    draw_afc_f(43, 44);             // Средний расход топлива суточный и общий (F)	
    draw_O2_sensor_h(86, 0);        // AFR (Напряжение УДК) (H)	
    draw_inj_corr_h(86, 10);        // Коррекция времени впрыска (H)	
    draw_airtemp_h(86, 22);         // Температура воздуха на впуске (H)	
    draw_battery_h(86, 32);         // Напряжение сети (H)	
    draw_fuel_burned_f(86, 44);     // Израсходованное топливо (F)	
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
    draw_rpm_f(0, 0);               // Обороты (F)	
    draw_water_temp_f(0, 22);       // Температура ОЖ (F)	
    draw_speed_f(0, 44, 0);         // Скорость с ЭБУ (F)	
    draw_map_f(43, 0);              // ДАД (F)	
    draw_trottle_f(43, 22);         // ДПДЗ / РХХ (F)	
    draw_speed_f(43, 44, 1);        // Скорость напрямую с ДС (F)	
    draw_oil_pressure_f(86, 0);     // Давление масла (F)	
    draw_airtemp_h(86, 22);         // Температура воздуха на впуске (H)	
    draw_angle_h(86, 32);           // Текущий УОЗ (H)	
    draw_air_flow_f(86, 44);        // Расход воздуха (F)	
	// ========================== Блоки данных ==========================
}

// Экран замера разгона.
static void display_draw_acceleration() {
	// Линии разметки
	oled_draw_h_line(86, 21, 42);
	oled_draw_h_line(86, 43, 42);
	oled_draw_v_line(85, 0, 64);

	// ========================== Блоки данных ==========================
    draw_speed_f(86, 0, 1);         // Скорость напрямую с ДС (F)	
    draw_map_f(86, 22);             // ДАД (F)	
    draw_water_temp_f(86, 44);      // Температура ОЖ (F)	
	// ========================== Блоки данных ==========================

	draw_acceleration();
}

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
		case DISPLAY_MAIN:
			display_draw_main();
			if (BK.ConfigBoxTimer) {draw_config_box(43, 22);}
			break;
		case DISPLAY_SECOND:
			display_draw_second();
			break;
		case DISPLAY_ACCELERATION:
			display_draw_acceleration();
			break;
		case DISPLAY_GRAPH:
			display_draw_graph();
			break;
		case DISPLAY_CURRENT_ERRORS:
			draw_current_errors();
			break;
		case DISPLAY_SAVED_ERRORS:
			draw_saved_errors();
			break;
		case DISPLAY_ADC:
			draw_adc_value();
			break;
		case DISPLAY_STATISTICS:
			draw_statistics();
			break;
	}

	draw_error_box();	// Индикация ошибок.
	oled_send_buffer();
}

void display_draw_no_signal() {
	oled_clear_buffer();

	// Линии разметки
	oled_draw_h_line(0, 21, 128);
	oled_draw_h_line(0, 43, 128);
	oled_draw_v_line(42, 0, 21);
	oled_draw_v_line(85, 0, 21);

	oled_draw_v_line(42, 44, 20);
	oled_draw_v_line(85, 44, 20);

	draw_distance_f(0, 44);			// Суточный и общий пробег (F)	
	draw_speed_f(43, 0, 1);			// Скорость напрямую с ДС (F)	
	draw_afc_f(43, 44);				// Средний расход топлива суточный и общий (F)	
	draw_fuel_burned_f(86, 44);		// Израсходованное топливо (F)	

	draw_no_signal();
	oled_send_buffer();
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