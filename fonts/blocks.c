// ����������� ����������
#include <stdint.h>			// �������� �������� int.
#include <stdio.h>			// ����������� ���������� �����/������.
#include <avr/pgmspace.h>	// ������ � PROGMEM.

#include "blocks.h"			// ���� ���������.
#include "macros.h"			// �������.
#include "oled.h"			// OLED �������.
#include "fonts.h"			// ������.
#include "pictograms.h"		// �������������.
#include "configuration.h"	// ���������.
#include "uart.h"			// UART.
#include "bkdata.h"			// ��������� � �������.
#include "dadm.h"			// ������ ���������� �������� �����.
#include "pinout.h"			// ���������� ������� �����������.
#include "buttons.h"		// ������.

#include "strings.h"		// ��������� ���������.

extern int16_t AlarmBoxTimer;		// �������� ������� �� main.

// ������ ��� ������ �� �����.
char Buffer[32] = {0};
const char NoSignal[] PROGMEM = "NO_SIGNAL";
const char NoErrors[] PROGMEM = "NO_ERRORS";

const char TravelTime[] PROGMEM = "TRAVEL_TIME";
const char TravelDist[] PROGMEM = "DISTANCE";
const char TravelFuel[] PROGMEM = "FUEL_CONSUMED";
const char TravelSpeed[] PROGMEM = "AVERAGE_SPEED";

void draw_error_box() {
    // �������� ����.
	#ifdef INT_OIL_PIN
		if (get_oil_pressure_state() && AlarmBoxTimer > 0) {
			oled_draw_box(0, 0, 128, 64, 1);
		}
	#endif
}

void draw_no_signal() {

	char Test[] = "����������";

	oled_set_font(u8g2_font_cyrillic_5x8_tr);
	oled_print_string(0, 1, "+01234-5678.9", 13);
	oled_print_string(0, 20, "��������������������", 20);
	oled_print_string(0, 40, "������������", 12);
	
	oled_set_font(u8g2_font_helvB08_tr);
	snprintf(Buffer, 6, "%i", Test[0]);
	oled_print_string(0, 50, Buffer, 5);


	// static uint8_t Counter = 0;

	// oled_set_font(u8g2_font_helvB08_tr);
	// oled_print_string_f(29, 28, NoSignal, 9);

	// snprintf(Buffer, 4, "%3i", uart_get_rx_status());
	// oled_print_string(50, 40, Buffer, 3);

	// for (uint8_t i = 0; i < 7; i++) {
	// 	snprintf(Buffer, 3, "%X", get_rx_buffer(i));
	// 	oled_print_string(17 * i + 4, 10, Buffer, 2);
	// }

	// snprintf(Buffer, 4, "%3i", BK.DataError);
	// oled_print_string(20, 50, Buffer, 2);


	// oled_draw_xbmp(25, 50, TT_bits, TT_width, TT_height);

	// snprintf(Buffer, 4, "%3i", Counter);
	// oled_print_string(80, 50, Buffer, 3);
	// Counter++;
}

void draw_water_temp_f(uint8_t x, uint8_t y) {
	int8_t TEMP = uart_get_uint8(11) - 40;
	TEMP = MAX(-99, TEMP);
	
	// ������ ����������� ��������� � ���� �������.
	oled_draw_xbmp(x + 2, y + 3, WaterTempL_bits, WaterTempL_width, WaterTempL_height);
	oled_draw_xbmp(x + 37, y + 4, Cels_bits, Cels_width, Cels_height);

	snprintf(Buffer, 4, "%3i", TEMP);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);
	oled_print_string(x + 11, y + 5, Buffer, 3);

	// ������ �������������, ��� ������ ��������� �� ���������� �������.
	if (AlarmBoxTimer > 0) {
		if (TEMP < WATER_TEMP_MIN || TEMP > WATER_TEMP_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_O2_sensor_f(uint8_t x, uint8_t y) {
	// ��� ������� ���������, 0 - ���, 1 - ���.
	uint8_t LambdaType;
	uint16_t AFR = (147 * (uart_get_uint8(12) + 128)) >> 8; // �������� x10.
	if (AFR > 0) {
		// ���� ���� ��������� ���, ������ ���������� ���� ���.
		LambdaType = 1;
		snprintf(Buffer, 5, "%2u.%1u", AFR / 10, AFR % 10);
	}
	else {
		// ����� ���������� ����������.
		LambdaType = 0;
		AFR = (125 * uart_get_uint8(23)) >> 8; // �������� x100.
		snprintf(Buffer, 5, "%1u.%02u", AFR / 100, AFR % 100);
	}

	oled_draw_xbmp(x + 2, y + 2, Lambda_L_bits, Lambda_L_width, Lambda_L_height);
	oled_set_font(u8g2_font_helvB10_tn);
	oled_print_string(x + 10, y + 5, Buffer, 4);

	if (AlarmBoxTimer > 0) {
		if (LambdaType == 1) {
			if (AFR < LAMBDA_AFR_MIN || AFR > LAMBDA_AFR_MAX) {
				oled_draw_box(x + 1, y + 1, 40, 19,  1);
			}
		}
		else {
			if (AFR < UDK_VOLT_MIN || AFR > UDK_VOLT_MAX) {
				oled_draw_box(x + 1, y + 1, 40, 19, 1);
			}
		}
	}
}

void draw_O2_sensor_h(uint8_t x, uint8_t y) {
	// ��� ������� ���������, 0 - ���, 1 - ���.
	uint8_t LambdaType;
	uint16_t AFR = (147 * (uart_get_uint8(12) + 128)) >> 8; // �������� x10.
	if (AFR > 0) {
		// ���� ���� ��������� ���, ������ ���������� ���� ���.
		LambdaType = 1;
		snprintf(Buffer, 5, "%2u.%1u", AFR / 10, AFR % 10);
	}
	else {
		// ����� ���������� ����������.
		LambdaType = 0;
		AFR = (125 * uart_get_uint8(23)) >> 8; // �������� x100.
		snprintf(Buffer, 5, "%1u.%02u", AFR / 100, AFR % 100);
	}

	oled_draw_xbmp(x + 2, y + 1, Lambda_S_bits, Lambda_S_width, Lambda_S_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);
	oled_print_string(x + 20, y + 2, Buffer, 4);

	if (AlarmBoxTimer > 0) {
		if (LambdaType == 1) {
			if (AFR < LAMBDA_AFR_MIN || AFR > LAMBDA_AFR_MAX) {
				oled_draw_box(x + 1, y + 1, 40, 9, 1);
			}
		}
		else {
			if (AFR < UDK_VOLT_MIN || AFR > UDK_VOLT_MAX) {
				oled_draw_box(x + 1, y + 1, 40, 9, 1);
			}
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
	oled_print_string(x + 18, y + 6, Buffer, 3);
}

void draw_rpm_f(uint8_t x, uint8_t y) {
	uint16_t RPM = uart_get_uint8(14) * 40;
	if (RPM < 2000) {RPM = uart_get_uint8(15) * 10;} // ��� ��.
	RPM = MIN(9999, RPM);

	oled_set_font(u8g2_font_helvB12_tn);
	snprintf(Buffer, 5, "%4u", RPM);
	oled_print_string(x + 2, y + 4, Buffer, 4);

	if (AlarmBoxTimer > 0) {
		if (RPM > RPM_LIMIT) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_map_f(uint8_t x, uint8_t y) {
	uint16_t MAP = uart_get_uint8(166);

	if (AlarmBoxTimer > 0) {
		if (MAP > OVERBOOST_LIMIT) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}

	oled_draw_xbmp(x + 2, y + 2, MapL_bits, MapL_width, MapL_height);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);
	snprintf(Buffer, 4, "%3u", MAP);
	oled_print_string(x + 14, y + 5, Buffer, 3);
}

// ��������� ������� �������.
void draw_inj_corr_h(uint8_t x, uint8_t y) {
	int16_t CORR = (uart_get_uint8(18) - 128) * 100; // x256

	oled_draw_xbmp(x + 2, y + 1, LambdaCorrS_bits, LambdaCorrS_width, LambdaCorrS_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 6, "%3i.%1u", CORR / 256, ABS(((CORR % 256) * 10) / 256));
	if (CORR > 0) {Buffer[0] = '+';}
	oled_print_string(x + 14, y + 2, Buffer, 5);

	if (AlarmBoxTimer > 0) {
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
	oled_print_string(x + 17, y + 2, Buffer, 4);
	oled_draw_xbmp(x + 37, y + 2, Cels_bits, Cels_width, Cels_height);
}

void draw_speed_f(uint8_t x, uint8_t y) {
	uint8_t Speed = uart_get_uint8(20);

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

	if (AlarmBoxTimer > 0) {
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

	if (AlarmBoxTimer > 0) {
		if (BAT < BATT_VOLT_MIN || BAT > BATT_VOLT_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 9, 1);
		}
	}
}

void draw_ff_fc_f(uint8_t x, uint8_t y) {
	uint8_t Speed = uart_get_uint8(20);
	// ������� ������ ������� �/� (x50).
	uint16_t FuelFlow = uart_get_uint16(31);

	// ����� ����������� � ����������� �� ��������, �/� ��� �/100��.
	oled_draw_xbmp(x + 2, y + 2, LETTER_F_bits, LETTER_F_width, LETTER_F_height);
	if (Speed <= 3) {
		oled_draw_xbmp(x + 2, y + 11, LETTER_F_bits, LETTER_F_width, LETTER_F_height);
		snprintf(Buffer, 5, "%2u.%1u", MIN(99, FuelFlow / 50), ((FuelFlow % 50) * 10) / 50);
	}
	else {
		// ������� ������ ������� �/100�� (x128).
		FuelFlow = uart_get_uint16(31);
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

	if (AlarmBoxTimer > 0) {
		if (AIRTEMP < AIR_TEMP_MIN || AIRTEMP > AIR_TEMP_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 9, 1);
		}
	}
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

	// ������� ������ ��������
	if (BK.DistDay + BK.DistRide > 1000) {
		AFC = (uint32_t) (BK.FuelDay + BK.FuelRide)* 100 / ((BK.DistDay + BK.DistRide) >> 7);		// x128
	}
	// ������� ������ �����
	if (BK.DistAll + BK.DistRide > 1000) {
		TAFC = (uint32_t) (BK.FuelAll + BK.FuelRide) * 100 / ((BK.DistAll + BK.DistRide) >> 7);	// x128
	}

	oled_draw_xbmp(x + 3, y + 3, AFC_bits, AFC_width, AFC_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 7, "%3u.%02u", AFC / 128, ((AFC % 128) * 100) / 128);
	oled_print_string(x + 7, y + 2, Buffer, 7);

	snprintf(Buffer, 7, "%3u.%02u", TAFC / 128, ((TAFC % 128) * 100) / 128);
	oled_print_string(x + 7, y + 11, Buffer, 7);
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

void draw_ce_errors() {
	#define CE_ROWS_ON_SCREEN 5
	#define CE_ROW_SPACE 4

	uint8_t ErrorsCount = uart_get_uint8(2);
	if (!ErrorsCount) {oled_print_string_f(29, 28, NoErrors, 9);}
	else {
		static uint8_t StartRow = 0;	// ����� ������ ������.

		// ����� ��������� ������ ��� ������������ ������.
		if (ButtonState[2] == 201 || ButtonState[3] == 201) {StartRow = 0;}

		oled_set_font(u8g2_font_helvB08_tr);
		uint8_t Col = 0;
		uint8_t Row = 0;
		for (uint8_t i = StartRow; i < ErrorsCount; i++) {
			if (Row < CE_ROWS_ON_SCREEN) {
				uint8_t StartByte = 3 + i * 3;
				uint8_t x = Col * 55 + 10;
				uint8_t y = Row * (CE_ROW_SPACE + 8) + 3;

				snprintf(Buffer, 6, "P%02X%02X", 
								uart_get_uint8(StartByte), 
								uart_get_uint8(StartByte + 1));

				oled_print_string(x, y, Buffer, 10);
				Row++;
				if (Col == 0 && Row == CE_ROWS_ON_SCREEN) {	// ������ �������.
					Col = 1;
					Row = 0;
				}
			}
		}

		// ���� ������ ����.
		if (StartRow) {oled_draw_xbmp(110, 31 - 7 - 5, Prev_bits, Prev_width, Prev_height);}
		// �������� �����.
		if (ButtonState[0] == 201 && StartRow) {StartRow -= CE_ROWS_ON_SCREEN * 2;}
		if (CE_ROWS_ON_SCREEN + StartRow + Col * CE_ROWS_ON_SCREEN < ErrorsCount) {
			// ���� ������ ����.
			oled_draw_xbmp(110, 31 + 5, Next_bits, Next_width, Next_height);
			if (ButtonState[1] == 201) {StartRow += CE_ROWS_ON_SCREEN * 2;}	// �������� ����.
		}

	}
}

void draw_adc_value() {
	#define ADC_ROWS_ON_SCREEN 4
	#define ADC_ROW_SPACE 5
	#define ADC_PARAMETERS_COUNT 7

	static uint8_t StartRow = 0;	// ����� ������ ������.

	// ����� ��������� ������ ��� ������������ ������.
	if (ButtonState[2] == 201 || ButtonState[3] == 201) {StartRow = 0;}

	oled_set_font(u8g2_font_helvB08_tr);
	uint8_t Row = 0;
	uint16_t Value = 0;
	uint8_t y = 0;
	for (uint8_t i = StartRow; i < ADC_PARAMETERS_COUNT; i++) {
		if (Row < ADC_ROWS_ON_SCREEN) {
			y = Row * (ADC_ROW_SPACE + 8) + 3;

			oled_print_string_f(1, y, (char*)pgm_read_word(&(ADCItemsArray[i])), 11);
			Value = uart_get_uint8(3 + i) * 5;
			snprintf(Buffer, 21, "%1u.%03lu", Value / 256, (uint32_t) (Value % 256) * 1000 / 256);
			oled_print_string(90, y, Buffer, 10);
			Row++;
		}
	}

	// ���� ������ ����.
	if (StartRow) {oled_draw_xbmp(72 - 5, 64 - 8, Prev_bits, Prev_width, Prev_height);}
	// �������� �����.
	if (ButtonState[0] == 201 && StartRow) {StartRow--;}
	if (ADC_ROWS_ON_SCREEN + StartRow < ADC_PARAMETERS_COUNT) {
		// ���� ������ ����.
		oled_draw_xbmp(56 - 5, 64 - 8, Next_bits, Next_width, Next_height);
		// �������� ����.
		if (ButtonState[1] == 201) {StartRow++;}
	}
}

/*
void draw_inj_duty_f(uint8_t x, uint8_t y) {
	uint8_t InjDuty = get_byte(81) >> 1;	// x2

	oled_draw_xbmp(x + 2, y + 3, InjDuty_bits, InjDuty_width, InjDuty_height);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);

	snprintf(Buffer, 4, "%3u", InjDuty);
	oled_print_string(x + 12, y + 4, Buffer, 3);
}

void draw_fan_pwm_f(uint8_t x, uint8_t y) {
	uint8_t FanPWM = get_byte(84) >> 1;		// x2

	oled_draw_xbmp(x + 1, y + 2, FAN_bits, FAN_width, FAN_height);
	oled_set_font(u8g2_font_helvB12_tn);

	snprintf(Buffer, 4, "%3u", FanPWM);
	oled_print_string(x + 12, y + 4, Buffer, 3);
}







void draw_statistics() {
	#define ROW_SPACE_FINISH 16
	#define ROW_SHIFT 5

	BK.RideTimer = 6160;
	BK.DistRide = 190950;
	BK.FuelRide = 10750;

	uint8_t Row = 0;
	uint8_t y = 0;
	oled_set_font(u8g2_font_helvB08_tr);
	// ������ 1 - ����� ������� � �������
	Row = 1;
	snprintf(Buffer, 6, "%3u.%1u", MIN(999, BK.RideTimer / 60), ((BK.RideTimer % 60) * 10) / 60);
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(1, y, TravelTime, 11);
	oled_print_string(100, y, Buffer, 5);

	// ������ 2 - ���������� ����������
	Row = 2;
	snprintf(Buffer, 7, "%4lu.%1lu", MIN(9999, BK.DistRide / 1000), ((BK.DistRide % 1000) * 10) / 1000);
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(1, y, TravelDist, 8);
	oled_print_string(94, y, Buffer, 6);

	// ������ 3 - ������������� �������
	Row = 3;
	snprintf(Buffer, 6, "%3lu.%1lu", MIN(999, BK.FuelRide / 1000), ((BK.FuelRide % 1000) * 10) / 1000);
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(1, y, TravelFuel, 13);
	oled_print_string(100, y, Buffer, 5);

	// ������ 4 - ������� ��������
	Row = 4;
	uint32_t SpeedTmp = (BK.DistRide * 36 / 10);
	snprintf(Buffer, 6, "%3lu.%1lu", MIN(999, SpeedTmp / BK.RideTimer), MIN(9, ((SpeedTmp % BK.RideTimer) * 10) / BK.RideTimer));
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(1, y, TravelSpeed, 13);
	oled_print_string(100, y, Buffer, 5);
}



	//char BufferT[] = "+01234-5678.9";
	//char BufferT[] = "ABCDEFGHIJKLMNO";
	//char BufferT[] = "PQRSTUVWXYZ----";
	//char_shift(BufferT, 15);
	//oled_print_string(0, 42, Buffer, 15);s

PORTB ^= (1 << 5);
*/
