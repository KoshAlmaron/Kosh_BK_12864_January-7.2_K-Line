// Подключение бибилиотек
#include <stdint.h>			// Коротние название int.
#include <stdio.h>			// Стандартная библиотека ввода/вывода.
#include <avr/pgmspace.h>	// Работа с PROGMEM.

#include "blocks.h"			// Свой заголовок.
#include "macros.h"			// Макросы.
#include "oled.h"			// OLED дисплей.
#include "fonts.h"			// Шрифты.
#include "pictograms.h"		// Пиктограммамы.
#include "configuration.h"	// Настройки.
#include "uart.h"			// UART.
#include "bkdata.h"			// Структура с данными.
#include "dadm.h"			// Датчик аварийного давления масла.
#include "pinout.h"			// Начначение выводов контроллера.
#include "buttons.h"		// Кнопки.
#include "adc.h"			// АЦП.
#include "spdsens.h"		// Датчик скорости.



#include "strings.h"		// Строковые константы.

static char Buffer[32] = {0};			// Строка для вывода на экран.

// Минимальное и максимальное значение.
const uint16_t GraphParameters[][2] PROGMEM =  {
		  {0, 1 * 100}		// НАПРЯЖЕНИЕ_УДК		x100
		, {100, 200 * 1}	// ДАВЛЕНИЕ_ВПУСКА		x1
		, {0, 700 * 6}		// ЦИКЛОВОЕ_НАПОЛ-Е		x6
		, {0, 550 * 10}		// РАСХОД_ВОЗДУХА		x10
		, {0, 30 * 125}		// ВРЕМЯ_ВПРЫСКА		x125
		, {0, 40 * 2}		// ТЕКУЩИЙ_УОЗ			x2
		, {600, 2000 * 1}	// ОБОРОТЫ_ХХ			x1
		, {0, 6 * 10}		// ДАВЛЕНИЕ_МАСЛА		x10
		, {0, 255 * 1}		// ПОЛОЖЕНИЕ_РХХ		x1
};

const uint8_t AccelPosY[3] = {5, 26, 49};

void draw_error_box() {
    // Проверка ДАДМ.
	#ifdef INT_OIL_PIN
		if (BK.ScreenMode < 5) {
			uint16_t RPM = uart_get_uint8(14) * 40;
			if (dadm_get_state(RPM) && BK.AlarmBoxTimer > 0) {
				oled_draw_box(0, 0, 128, 64, 1);
			}
		}
	#endif
}

void draw_no_signal() {
	oled_set_font(u8g2_font_cyrillic_5x8_tr);
	oled_print_string_f(32, 29, NoSignal, 11);

	#ifdef DEBUG_MODE
		oled_set_font(u8g2_font_helvB08_tr);
		static uint8_t Counter = 0;

		for (uint8_t i = 0; i < 9; i++) {
			snprintf(Buffer, 3, "%02X", get_rx_buffer(i));
			oled_print_string(14 * i + 2, 10, Buffer, 2);
		}

		snprintf(Buffer, 4, "%3i", BK.DataError);
		oled_print_string(3, 50, Buffer, 3);

		snprintf(Buffer, 4, "%3i", uart_get_rx_status());
		oled_print_string(54, 50, Buffer, 3);

		snprintf(Buffer, 4, "%3i", Counter);
		oled_print_string(100, 50, Buffer, 3);
		Counter++;
	#endif
}

void draw_config_box(uint8_t x, uint8_t y) {
	oled_draw_box(x, y, 42, 21, 0);
	oled_draw_mode(1);
	
	#ifdef AUTO_BRIGHT_ADC_CHANNEL
		uint8_t Value = get_adc_value() >> 2;

		oled_set_font(u8g2_font_haxrcorp4089_tn);
		snprintf(Buffer, 8, "%03u-%03u", Value, BK.BrightLCD[0]);
		oled_print_string(x + 1, y + 2, Buffer, 7);

		snprintf(Buffer, 8, "%03u-%03u", BK.BrightLCD[1], BK.BrightLCD[2]);
		oled_print_string(x + 1, y + 12, Buffer, 7);
	#else
		oled_set_font(u8g2_font_pxplusibmvga8_tn);
		snprintf(Buffer, 4, "%3u", BK.BrightLCD[0]);
		oled_print_string(x + 10, y + 5, Buffer, 3);
	#endif
	oled_draw_mode(0);
}

void draw_acceleration() {
	oled_set_font(u8g2_font_helvB08_tr);

	// snprintf(Buffer, 4, "%-3u", BK.AccelMeterStatus);
	// oled_print_string(0, 0, Buffer, 3);

	switch (BK.AccelMeterStatus) {
		case 0:
			break;
		case 1:
			oled_draw_xbmp(16, 26, Stop_bits, Stop_width, Stop_height);
			break;
		case 2:
			oled_draw_xbmp(16, 26, Stop_bits, Stop_width, Stop_height);
			break;
		case 3:
			oled_draw_xbmp(8, 26, Ready_bits, Ready_width, Ready_height);
			break;
		case 4:
			oled_draw_xbmp(8, 26, Ready_bits, Ready_width, Ready_height);
			break;
		case 64:
			oled_draw_xbmp(2, 26, Fail_bits, Fail_width, Fail_height);
			break;
		default:
			if (BK.AccelMeterStatus == 5 || BK.AccelMeterStatus == 7) {
				oled_print_string(6, 7, "0-30", 4);
				oled_print_string(6, 28, "0-60", 4);
				oled_print_string(6, 51, "0-100", 5);
				oled_set_font(u8g2_font_helvB10_tn);
				for (uint8_t i = 0; i < 3; i++) {
					if (get_accel_time(i)) {
						snprintf(Buffer, 6, "%2u.%02u", get_accel_time(i) / 1000, (get_accel_time(i) % 1000) * 100 / 1000);
						oled_print_string(38, AccelPosY[i], Buffer, 5);
					}
				}					
			}
			else if (BK.AccelMeterStatus == 6 || BK.AccelMeterStatus == 8) {
				oled_print_string(6, 28, "60-100", 6);
				oled_set_font(u8g2_font_helvB10_tn);
				if (get_accel_time(2)) {
					snprintf(Buffer, 6, "%2u.%02u", get_accel_time(2) / 1000, (get_accel_time(2) % 1000) * 100 / 1000);
					oled_print_string(38, AccelPosY[1], Buffer, 5);
				}
			}
			break;
	}


	// 0 - Отключено,
	// 1 - ожидание остановки,
	// 2 - ожидание скорости < 50 км/ч
	// 3 - готов к замеру 0-100,
	// 4 - готов к замеру 60-100,
	// 5 - замер 0-100 запущен,
	// 6 - замер 60-100 запущен,
	// 7 - замер 0-100 завершен,
	// 8 - замер 60-100 завершен,
	// 64 - ошибка.

	
}

void draw_current_errors() {
	#define CE_ROWS_ON_SCREEN 4
	#define CE_ROW_SPACE 5
	
	static uint8_t StartRow = 0;	// Номер первой строка.

	uint8_t ErrorsCount = 0;
	for (uint8_t N = 0; N < 4; N++) {
		if (uart_get_uint8(7 + N)) {ErrorsCount++;}
	}
	
	if (!ErrorsCount) {
		oled_set_font(u8g2_font_cyrillic_5x8_tr);
		oled_print_string_f(43, 23, CurrentErrors, 7);
		oled_print_string_f(36, 35, NoErrors, 10);
	}
	else {
		oled_set_font(u8g2_font_cyrillic_5x8_tr);
		uint8_t Row = 0;
		uint8_t OverRow = 0;
		uint8_t y = 0;
		for (uint8_t N = 0; N < 4; N++) {		// 4 байта с флагами ошибок.
			for (uint8_t i = 0; i < 8; i++) {	// 8 бит.
				if (BITREAD(uart_get_uint8(7 + N), i)) {	// Бит установлен.
					if (Row - StartRow < CE_ROWS_ON_SCREEN) {
						if (Row >= StartRow) {
							y = (Row - StartRow) * (CE_ROW_SPACE + 8) + 3;
							oled_print_string_f(1, y, (char*)pgm_read_word(&(CEItemsArray[N * 8 + i])), 20);
						}
						Row++;
					}
					else {
						OverRow = 1;
					}
				}
			}
		}

		// Есть строки выше.
		if (StartRow) {oled_draw_xbmp(72 - 5, 64 - 8, Prev_bits, Prev_width, Prev_height);}
		// Смещение вверх.
		if (buttons_get_state(BTN_UP) == 1 && StartRow) {StartRow--;}
		if (OverRow) {
			// Есть строки ниже.
			oled_draw_xbmp(56 - 5, 64 - 8, Next_bits, Next_width, Next_height);
			// Смещение вниз.
			if (buttons_get_state(BTN_DOWN) == 1) {StartRow++;}
		}
	}
	// Сброс начальной строки при переключении экрана.
	if (buttons_get_state(BTN_LEFT) == 1 || buttons_get_state(BTN_RIGHT) == 1) {StartRow = 0;}
}

void draw_saved_errors() {
	#define SE_ROWS_ON_SCREEN 5
	#define SE_ROW_SPACE 4

	static uint8_t StartRow = 0;	// Номер первой строка.

	uint8_t ErrorsCount = uart_get_uint8(2);
	if (!ErrorsCount) {
		oled_set_font(u8g2_font_cyrillic_5x8_tr);
		oled_print_string_f(31, 23, SavedErrors, 11);
		oled_print_string_f(36, 35, NoErrors, 10);
	}
	else {
		oled_set_font(u8g2_font_helvB08_tr);
		uint8_t Col = 0;
		uint8_t Row = 0;
		for (uint8_t i = StartRow; i < ErrorsCount; i++) {
			if (Row < SE_ROWS_ON_SCREEN) {
				uint8_t StartByte = 3 + i * 3;
				uint8_t x = Col * 55 + 10;
				uint8_t y = Row * (SE_ROW_SPACE + 8) + 3;

				snprintf(Buffer, 6, "P%02X%02X",
								uart_get_uint8(StartByte), 
								uart_get_uint8(StartByte + 1));

				oled_print_string(x, y, Buffer, 5);
				Row++;
				if (Col == 0 && Row == SE_ROWS_ON_SCREEN) {	// Вторая колонка.
					Col = 1;
					Row = 0;
				}
			}
		}

		// Есть строки выше.
		if (StartRow) {oled_draw_xbmp(110, 31 - 7 - 5, Prev_bits, Prev_width, Prev_height);}
		// Смещение вверх.
		if (buttons_get_state(BTN_UP) == 1 && StartRow) {StartRow -= SE_ROWS_ON_SCREEN * 2;}
		if (SE_ROWS_ON_SCREEN + StartRow + Col * SE_ROWS_ON_SCREEN < ErrorsCount) {
			// Есть строки ниже.
			oled_draw_xbmp(110, 31 + 5, Next_bits, Next_width, Next_height);
			if (buttons_get_state(BTN_DOWN) == 1) {StartRow += SE_ROWS_ON_SCREEN * 2;}	// Смещение вниз.
		}
	}

	// Сброс начальной строки при переключении экрана.
	if (buttons_get_state(BTN_LEFT) == 1 || buttons_get_state(BTN_RIGHT) == 1) {StartRow = 0;}
}

void draw_adc_value() {
	#define ADC_ROWS_ON_SCREEN 4
	#define ADC_ROW_SPACE 5
	#define ADC_PARAMETERS_COUNT 7

	static uint8_t StartRow = 0;	// Номер первой строки.

	oled_set_font(u8g2_font_cyrillic_5x8_tr);
	uint8_t Row = 0;
	uint16_t Value = 0;
	uint8_t y = 0;
	for (uint8_t i = StartRow; i < ADC_PARAMETERS_COUNT; i++) {
		if (Row < ADC_ROWS_ON_SCREEN) {
			y = Row * (ADC_ROW_SPACE + 8) + 3;

			oled_print_string_f(1, y, (char*)pgm_read_word(&(ADCItemsArray[i])), 8);
			Value = uart_get_uint8(3 + i) * 5;
			snprintf(Buffer, 6, "%1u.%03lu", Value / 256, (uint32_t) (Value % 256) * 1000 / 256);
			oled_print_string(90, y, Buffer, 5);
			Row++;
		}
	}

	// Есть строки выше.
	if (StartRow) {oled_draw_xbmp(72 - 5, 64 - 8, Prev_bits, Prev_width, Prev_height);}
	// Смещение вверх.
	if (buttons_get_state(BTN_UP) == 1 && StartRow) {StartRow--;}
	if (ADC_ROWS_ON_SCREEN + StartRow < ADC_PARAMETERS_COUNT) {
		// Есть строки ниже.
		oled_draw_xbmp(56 - 5, 64 - 8, Next_bits, Next_width, Next_height);
		// Смещение вниз.
		if (buttons_get_state(BTN_DOWN) == 1) {StartRow++;}
	}

	// Сброс начальной строки при переключении экрана.
	if (buttons_get_state(BTN_LEFT) == 1 || buttons_get_state(BTN_RIGHT) == 1) {StartRow = 0;}
}

void draw_graph(uint8_t GraphNumber) {
	#define ROWS_COUNT 53
	static uint8_t Space = 0;
	static uint8_t PrevNumber = 0xff;
	static int16_t PrevY = 0;

	oled_draw_box(103, 0, 24, 9, 2); // Стираем место под значение параметра.

	oled_set_font(u8g2_font_cyrillic_5x8_tr);
	oled_print_string_f(1, 0, (char*)pgm_read_word(&(GraphItemsArray[GraphNumber])), 16);

	uint16_t Values[6] = {0}; // Значение, Мин, 1/4, 1/2, 3/4, Макс.
	uint8_t PosY[6] = {0, 57, 47, 34, 21, 11};

	Values[1] = pgm_read_word(&GraphParameters[GraphNumber][0]);		// 0.
	Values[5] = pgm_read_word(&GraphParameters[GraphNumber][1]);		// 1.

	Values[2] = (Values[5] - Values[1]) / 4 + Values[1];				// 1/4.
	Values[3] = (Values[5] - Values[1]) / 2 + Values[1];				// 1/2.
	Values[4] = (Values[5] - Values[1]) * 3 / 4 + Values[1];			// 3/4.

	uint16_t Numerator = (Values[5] - Values[1]) * 10 / ROWS_COUNT;		// Делитель.

	oled_set_font(u8g2_font_haxrcorp4089_tn);
	uint8_t Len = 0;
	switch (GraphNumber) {
		case 0:	// НАПРЯЖЕНИЕ_УДК
			Len = 4;
			Values[0] = (125 * uart_get_uint8(23)) >> 8; // x100.
			for (uint8_t i = 0; i < 6; i++) {
				snprintf(Buffer + Len * i, 5, "%1u.%02u", MIN(1, Values[i] / 100), MIN(99, Values[i] % 100));
			}
			break;
		case 1:	// ДАВЛЕНИЕ_ВО_ВПУСКЕ
			Len = 3;
			Values[0] = uart_get_uint8(16);			// x1
			for (uint8_t i = 0; i < 6; i++) {
				snprintf(Buffer + Len * i, 4, "%3u", MIN(999, Values[i]));
			}
			break;
		case 2:	// ЦИКЛОВОЕ_НАПОЛНЕНИЕ
			Len = 3;
			Values[0] = uart_get_uint16(29);		// x6
			for (uint8_t i = 0; i < 6; i++) {
				snprintf(Buffer + Len * i, 4, "%3u", MIN(999, Values[i] / 6));
			}
			break;
		case 3:	// РАСХОД_ВОЗДУХА
			Len = 3;
			Values[0] = uart_get_uint16(27);		// x10
			for (uint8_t i = 0; i < 6; i++) {
				snprintf(Buffer + Len * i, 4, "%3u", MIN(999, Values[i] / 10));
			}
			break;
		case 4:	// ВРЕМЯ_ВПРЫСКА
			Len = 4;
			Values[0] = uart_get_uint16(25);		// x125
			for (uint8_t i = 0; i < 6; i++) {
				snprintf(Buffer + Len * i, 5, "%2u.%1u", MIN(99, Values[i] / 125), ((Values[i] % 125) * 10) / 125);
			}
			break;
		case 5:	// ТЕКУЩИЙ_УОЗ
			Len = 4;
			Values[0] = uart_get_int8(19);			// x2
			for (uint8_t i = 0; i < 6; i++) {
				snprintf(Buffer + Len * i, 5, "%2u.%1u", MIN(99, (Values[i] / 2)), MIN(9, ABS((( Values[i] % 2) * 10) / 2)));
			}
			break;
		case 6:	// ОБОРОТЫ_ХХ
			Len = 4;
			Values[0] = uart_get_uint8(15) * 10;	// x1
			for (uint8_t i = 0; i < 6; i++) {
				snprintf(Buffer + Len * i, 5, "%4u", Values[i]);
			}
			break;
		case 7:	// ДАВЛЕНИЕ_МАСЛА
			Len = 3;
			Values[0] = uart_get_uint8(22);			// x10
			for (uint8_t i = 0; i < 6; i++) {
				snprintf(Buffer + Len * i, 4, "%1u.%1u", MIN(9, Values[i] / 10), MIN(9, Values[i] % 10));
			}
			break;
		case 8:	// ПОЛОЖЕНИЕ_РХХ
			Len = 3;
			Values[0] = uart_get_uint8(17);			// x1
			for (uint8_t i = 0; i < 6; i++) {
				snprintf(Buffer + Len * i, 6, "%3u", MIN(999, Values[i]));
			}
			break;
	}

	for (uint8_t i = 0; i < 6; i++) {
		oled_print_string(105, PosY[i], Buffer + Len * i, Len);
	}

	if (Space == 12) {
		Space = 0;
		oled_draw_pixel(100, 11);
		oled_draw_pixel(100, 37);
		oled_draw_pixel(100, 63);

		oled_draw_pixel(100, 24);
		oled_draw_pixel(100, 50);
	}
	Space++;

	if (!BK.ScreenChange) {oled_shift_graph_block();}
	Values[0] = CONSTRAIN(Values[0], Values[1], Values[5]);
	uint8_t y = (int32_t) 63 - MAX(0, Values[0] - Values[1]) * 10 / Numerator;
	y = MAX(11, y);

	if (GraphNumber != PrevNumber) {	// Первый цикл.
		PrevNumber = GraphNumber;
		PrevY = y;
	}	
	oled_draw_v_line(100, MIN(y, PrevY), MAX(1, ABS(y - PrevY)));
	PrevY = y;
}

void draw_water_temp_f(uint8_t x, uint8_t y) {
	int8_t TEMP = uart_get_uint8(11) - 40;
	TEMP = MAX(-99, TEMP);
	
	// Рисуем пиктограмму параметра и знач градуса.
	oled_draw_xbmp(x + 2, y + 3, WaterTempL_bits, WaterTempL_width, WaterTempL_height);
	oled_draw_xbmp(x + 37, y + 4, Cels_bits, Cels_width, Cels_height);

	snprintf(Buffer, 4, "%3i", TEMP);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);
	oled_print_string(x + 13, y + 5, Buffer, 3);

	// Рисуем прямоугольник, при выходе параметра за допустимые пределы.
	if (BK.AlarmBoxTimer > 0) {
		if (TEMP < WATER_TEMP_MIN || TEMP > WATER_TEMP_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_O2_sensor_f(uint8_t x, uint8_t y) {
	uint16_t AFR = (uint16_t) (125 * uart_get_uint8(23)) >> 8; // Значение x100.
	snprintf(Buffer, 5, "%1u.%02u", AFR / 100, AFR % 100);

	oled_draw_xbmp(x + 2, y + 2, Lambda_L_bits, Lambda_L_width, Lambda_L_height);
	oled_set_font(u8g2_font_helvB10_tn);
	oled_print_string(x + 10, y + 5, Buffer, 4);

	if (BK.AlarmBoxTimer > 0) {
		if (AFR < UDK_VOLT_MIN || AFR > UDK_VOLT_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_O2_sensor_h(uint8_t x, uint8_t y) {
	uint16_t AFR = (uint16_t) (125 * uart_get_uint8(23)) >> 8; // Значение x100.
	snprintf(Buffer, 5, "%1u.%02u", AFR / 100, AFR % 100);

	oled_draw_xbmp(x + 2, y + 1, Lambda_S_bits, Lambda_S_width, Lambda_S_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);
	oled_print_string(x + 20, y + 2, Buffer, 4);

	if (BK.AlarmBoxTimer > 0) {
		if (AFR < UDK_VOLT_MIN || AFR > UDK_VOLT_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 9, 1);
		}
	}
}

void draw_trottle_f(uint8_t x, uint8_t y) {
	uint8_t TPS = uart_get_uint8(13);
	uint8_t IAC = uart_get_uint8(17);

	if (TPS > 2) {
		snprintf(Buffer, 6, "%3u", TPS);
		oled_draw_xbmp(x + 2, y + 4, Trottle_bits, Trottle_width, Trottle_height);
	}
	else {
		snprintf(Buffer, 6, "%3u", IAC);
		oled_draw_xbmp(x + 2, y + 4, IAC_bits, IAC_width, IAC_height);
	}
	
	oled_set_font(u8g2_font_helvB08_tr);
	oled_print_string(x + 17, y + 6, Buffer, 3);
}

void draw_rpm_f(uint8_t x, uint8_t y) {
	uint16_t RPM = uart_get_uint8(14) * 40;
	if (RPM < 2000) {RPM = uart_get_uint8(15) * 10;} // Для ХХ.
	RPM = MIN(9999, RPM);

	oled_set_font(u8g2_font_helvB12_tn);
	snprintf(Buffer, 5, "%4u", RPM);
	oled_print_string(x + 2, y + 4, Buffer, 4);

	if (BK.AlarmBoxTimer > 0) {
		if (RPM > RPM_LIMIT) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_map_f(uint8_t x, uint8_t y) {
	uint16_t MAP = uart_get_uint8(16);

	oled_draw_xbmp(x + 2, y + 2, MapL_bits, MapL_width, MapL_height);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);
	snprintf(Buffer, 4, "%3u", MAP);
	oled_print_string(x + 16, y + 5, Buffer, 3);

	if (BK.AlarmBoxTimer > 0) {
		if (MAP > OVERBOOST_LIMIT) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_inj_corr_h(uint8_t x, uint8_t y) {	// Коррекция времени впрыска.
	int16_t CORR = (int16_t) (uart_get_uint8(18) - 128) * 100; // x256

	oled_draw_xbmp(x + 2, y + 1, LambdaCorrS_bits, LambdaCorrS_width, LambdaCorrS_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 6, "%3i.%1u", CORR / 256, (((ABS(CORR) % 256) * 10) / 256));

	uint8_t SymbolPos = 1;
	if (uart_get_uint8(18) < 103 || uart_get_uint8(18) > 153) {SymbolPos = 0;}

	if (uart_get_uint8(18) > 128) {Buffer[SymbolPos] = '+';}
	else if (uart_get_uint8(18) < 128)  {Buffer[SymbolPos] = '-';}

	oled_print_string(x + 14, y + 2, Buffer, 5);

	if (BK.AlarmBoxTimer > 0) {
		if (CORR < LAMBDA_CORR_MIN || CORR > LAMBDA_CORR_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 9, 1);
		}
	}
}

void draw_angle_h(uint8_t x, uint8_t y) {
	int8_t ANGLE = uart_get_int8(19);	// x2

	oled_draw_xbmp(x + 2, y + 1, Angle_bits, Angle_width, Angle_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 6, "%3i.%1u", MAX(-99, (ANGLE / 2)), MIN(9, ABS(((ANGLE % 2) * 10) / 2)));
	oled_print_string(x + 10, y + 2, Buffer, 5);
	oled_draw_xbmp(x + 37, y + 2, Cels_bits, Cels_width, Cels_height);
}

void draw_speed_f(uint8_t x, uint8_t y, uint8_t Type) {
	uint8_t Speed = uart_get_uint8(20);
	if (Type == 1) {
		Speed = get_car_speed();
		oled_draw_xbmp(x + 1, y + 2, SpeedDirect_bits, SpeedDirect_width, SpeedDirect_height);
	}
	else {
		oled_draw_xbmp(x + 1, y + 2, Speed_bits, Speed_width, Speed_height);
	}

	oled_draw_xbmp(x + 1, y + 2, Speed_bits, Speed_width, Speed_height);
	oled_set_font(u8g2_font_helvB12_tn);

	snprintf(Buffer, 4, "%3u", Speed);
	oled_print_string(x + 12, y + 4, Buffer, 3);
}

void draw_battery_f(uint8_t x, uint8_t y) {
	uint16_t BAT = 52 + uart_get_uint8(21) / 2 ;	// x10
	snprintf(Buffer, 5, "%2u.%1u", BAT / 10, BAT % 10);

	oled_draw_xbmp(x + 2, y + 2, BatteryL_bits, BatteryL_width, BatteryL_height);
	oled_set_font(u8g2_font_helvB10_tn);
	oled_print_string(x + 10, y + 5, Buffer, 4);

	if (BK.AlarmBoxTimer > 0) {
		if (BAT < BATT_VOLT_MIN || BAT > BATT_VOLT_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_battery_h(uint8_t x, uint8_t y) {
	uint16_t BAT = 52 + uart_get_uint8(21) / 2 ;	// x10
	snprintf(Buffer, 5, "%2u.%1u", BAT / 10, BAT % 10);

	oled_draw_xbmp(x + 2, y + 1, BatteryS_bits, BatteryS_width, BatteryS_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);
	oled_print_string(x + 18, y + 2, Buffer, 4);

	if (BK.AlarmBoxTimer > 0) {
		if (BAT < BATT_VOLT_MIN || BAT > BATT_VOLT_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 9, 1);
		}
	}
}

void draw_ff_fc_f(uint8_t x, uint8_t y) {
	uint8_t Speed = uart_get_uint8(20);
	uint16_t FuelFlow = 0;

	// Режим отображения в зависимости от скорости.
	// Часовой расход топлива л/ч (x50).
	oled_draw_xbmp(x + 2, y + 2, LETTER_F_bits, LETTER_F_width, LETTER_F_height);
	if (Speed <= 3) {
		FuelFlow = uart_get_uint16(31);
		oled_draw_xbmp(x + 2, y + 11, LETTER_F_bits, LETTER_F_width, LETTER_F_height);
		snprintf(Buffer, 5, "%2u.%1u", MIN(99, FuelFlow / 50), ((FuelFlow % 50) * 10) / 50);
	}
	else {
		// Путевой расход топлива л/100км (x128).
		FuelFlow = uart_get_uint16(33);
		if (FuelFlow == 0xFFFF) {FuelFlow = 0;}
		oled_draw_xbmp(x + 2, y + 11, LETTER_C_bits, LETTER_C_width, LETTER_C_height);
		snprintf(Buffer, 5, "%2u.%1u", MIN(99, FuelFlow / 128), ((FuelFlow % 128) * 10) / 128);
	}
	
	oled_set_font(u8g2_font_helvB12_tn);
	oled_print_string(x + 8, y + 4, Buffer, 4);
}

void draw_airtemp_h(uint8_t x, uint8_t y) {
	int8_t AIRTEMP = uart_get_uint8(37) - 40;

	oled_draw_xbmp(x + 4, y + 1, AirTempS_bits, AirTempS_width, AirTempS_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 5, "%3i", AIRTEMP);
	oled_print_string(x + 18, y + 2, Buffer, 4);
	oled_draw_xbmp(x + 37, y + 2, Cels_bits, Cels_width, Cels_height);

	if (BK.AlarmBoxTimer > 0) {
		if (AIRTEMP < AIR_TEMP_MIN || AIRTEMP > AIR_TEMP_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 9, 1);
		}
	}
}

void draw_oil_pressure_f(uint8_t x, uint8_t y) {
	int8_t OilPress = uart_get_uint8(22);
	uint16_t RPM = uart_get_uint8(14) * 40;

	oled_set_font(u8g2_font_helvB10_tn);

	snprintf(Buffer, 4, "%1u.%1u", OilPress / 10, OilPress % 10);
	oled_print_string(x + 4, y + 5, Buffer, 3);
	oled_draw_xbmp(x + 29, y + 2, OilPressure_bits, OilPressure_width, OilPressure_height);

	if (BK.AlarmBoxTimer > 0 && RPM > 500) {
		if (OilPress < OIL_PRESSURE_MIN || OilPress > OIL_PRESSURE_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_pulse_widgh_f(uint8_t x, uint8_t y) {
	uint16_t PW = uart_get_uint16(25);

	oled_draw_xbmp(x + 2, y + 2, Injector_bits, Injector_width, Injector_height);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);

	snprintf(Buffer, 5, "%2u.%1u", MIN(99, PW / 125), ((PW % 125) * 10) / 125);
	oled_print_string(x + 14, y + 5, Buffer, 4);
}

void draw_gbc_f(uint8_t x, uint8_t y) {
	uint16_t GBC = uart_get_uint16(29);		// x6

	oled_draw_xbmp(x + 2, y + 3, GBC_bits, GBC_width, GBC_height);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);

	snprintf(Buffer, 4, "%3u", MIN(999, GBC / 6));
	oled_print_string(x + 16, y + 5, Buffer, 3);
}

void draw_air_flow_f(uint8_t x, uint8_t y) {
	uint16_t AirFlow = uart_get_uint16(27);	// x10

	oled_draw_xbmp(x + 1, y + 3, Air_Flow_bits, Air_Flow_width, Air_Flow_height);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);

	snprintf(Buffer, 4, "%3u", MIN(999, AirFlow / 10));
	oled_print_string(x + 16, y + 5, Buffer, 3);		
}

void draw_distance_f(uint8_t x, uint8_t y) {
	oled_draw_xbmp(x + 1, y + 2, Road_bits, Road_width, Road_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 7, "%4lu.%1lu", MIN(9999, (BK.DistDay + BK.DistRide) / 1000), \
								 (((BK.DistDay + BK.DistRide) % 1000) * 10) / 1000);
	oled_print_string(x + 9, y + 2, Buffer, 6);

	snprintf(Buffer, 7, "%6lu", MIN(999999, (BK.DistAll + BK.DistRide) / 1000));
	oled_print_string(x + 5, y + 11, Buffer, 6);
}

void draw_afc_f(uint8_t x, uint8_t y) {
	uint16_t AFC = 0;
	uint16_t TAFC = 0;

	// Средний расход суточный
	if (BK.DistDay + BK.DistRide > 1000) {
		AFC = (uint32_t) (BK.FuelDay + BK.FuelRide)* 100 / ((BK.DistDay + BK.DistRide) >> 7);		// x128
	}
	// Средний расход общий
	if (BK.DistAll + BK.DistRide > 1000) {
		TAFC = (uint32_t) (BK.FuelAll + BK.FuelRide) * 100 / ((BK.DistAll + BK.DistRide) >> 7);	// x128
	}

	oled_draw_xbmp(x + 3, y + 3, AFC_bits, AFC_width, AFC_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 7, "%3u.%02u", AFC / 128, ((AFC % 128) * 100) / 128);
	oled_print_string(x + 9, y + 2, Buffer, 7);

	snprintf(Buffer, 7, "%3u.%02u", TAFC / 128, ((TAFC % 128) * 100) / 128);
	oled_print_string(x + 9, y + 11, Buffer, 7);
}

void draw_fuel_burned_f(uint8_t x, uint8_t y) {
	oled_draw_xbmp(x + 2, y + 3, FuelCanister_bits, FuelCanister_width, FuelCanister_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 6, "%3lu.%1lu", MIN(999, (BK.FuelDay + BK.FuelRide) / 1000), \
								 (((BK.FuelDay + BK.FuelRide) % 1000) * 10) / 1000);
	oled_print_string(x + 14, y + 2, Buffer, 5);

	snprintf(Buffer, 7, "%4lu", MIN(9999, (BK.FuelAll + BK.FuelRide) / 1000));
	oled_print_string(x + 16, y + 11, Buffer, 6);
}

void draw_statistics() {
	#define ROW_SPACE_FINISH 16
	#define ROW_SHIFT 5

	//BK.RideTimer = 5160;
	//BK.DistRide = 190950;
	//BK.FuelRide = 10750;

	uint8_t Row = 0;
	uint8_t y = 0;
	oled_set_font(u8g2_font_cyrillic_5x8_tr);
	// Строка 1 - Время поездки в минутах
	Row = 1;
	snprintf(Buffer, 6, "%3u.%1u", MIN(999, BK.RideTimer / 60), ((BK.RideTimer % 60) * 10) / 60);
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(0, y, TravelTime, 12);
	oled_print_string(102, y, Buffer, 5);

	// Строка 2 - Пройденное расстояние
	Row = 2;
	snprintf(Buffer, 7, "%4lu.%1lu", MIN(9999, BK.DistRide / 1000), ((BK.DistRide % 1000) * 10) / 1000);
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(0, y, TravelDist, 9);
	oled_print_string(96, y, Buffer, 6);

	// Строка 3 - Израсходовано топлива
	Row = 3;
	snprintf(Buffer, 6, "%3lu.%1lu", MIN(999, BK.FuelRide / 1000), ((BK.FuelRide % 1000) * 10) / 1000);
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(0, y, TravelFuel, 14);
	oled_print_string(102, y, Buffer, 5);

	// Строка 4 - Средняя скорость
	Row = 4;
	uint32_t SpeedTmp = (BK.DistRide * 36 / 10);
	snprintf(Buffer, 6, "%3lu.%1lu", MIN(999, SpeedTmp / BK.RideTimer), MIN(9, ((SpeedTmp % BK.RideTimer) * 10) / BK.RideTimer));
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(0, y, TravelSpeed, 16);
	oled_print_string(102, y, Buffer, 5);
}



/*
	char BufferT[] = "+01234-5678.9";
	char BufferT[] = "ABCDEFGHIJKLMNO";
	char BufferT[] = "PQRSTUVWXYZ----";
	char_shift(BufferT, 15);
	oled_print_string(0, 42, Buffer, 15);s

PORTB ^= (1 << 5);

	oled_set_font(u8g2_font_cyrillic_5x8_tr);
	oled_print_string(0, 1, "+01234-5678.9", 13);
	oled_print_string(0, 20, "АБВГДЕЖЗИИКЛМНОПР", 17);
	oled_print_string(0, 40, "СТУФХЦЧШЩЪЫЬЭЮЯ", 15);

*/