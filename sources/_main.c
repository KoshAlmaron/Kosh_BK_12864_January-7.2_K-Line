// Подключение бибилиотек
#include <stdint.h>				// Коротние название int.
#include <avr/io.h>				// Названия регистров и номера бит.
#include <avr/interrupt.h>		// Прерывания.
#include <avr/wdt.h>			// Сторожевой собак.
#include "util/delay.h"			// Задержки.

#include "timers.h"				// Таймеры.
#include "macros.h"				// Макросы.
#include "pinout.h"				// Назначение выводов контроллера.

#ifdef SPI_CS_PIN
	#include "spi.h"			// SPI.
#else
	#include "i2c.h"			// I2C (TWI).
#endif

#include "oled.h"				// OLED дисплей.
#include "display.h"			// Экраны БК.

#include "uart.h"				// UART.
#include "bkdata.h"				// Структура с данными.
#include "configuration.h"		// Настройки.
#include "dadm.h"				// Датчик аварийного давления масла.
#include "eeprom.h"				// Чтение и запись EEPROM.
#include "buttons.h"			// Кнопки.
#include "counters.h"			// Счетчики пробега и расхода топлива.
#ifdef AUTO_BRIGHT_PIN
	#include "adc.h"				// АЦП.
#endif
#include "speed_chime.h"		// Колокольчик AE86.

// Основной счетчик времени,
// увеличивается по прерыванию на единицу каждую 1мс.
volatile uint8_t MainTimer = 0;

// Счетчики времени.
static uint16_t LCDTimer = 0;			// Обновление экрана.
static uint16_t ButtonsTimer = 0;		// Опрос кнопок.
static uint16_t RideTimerMs = 0;		// Таймер времени работы БК.
static int16_t AnswerTimer = 0;			// Таймер ожидания ответа.
static uint8_t AutoBrightTimer = 0;		// Таймер авто яркости.
static uint8_t SpeedChimeTimer = 0;		// Таймер колокольчика.
static uint16_t WaitTimer = 0;			// Таймер ожидания.

// Признак установленного соединения:
// 0 - требуется инициализация,
// 1 - ожидание начала соединения,
// 2 - соединение установлено.
static uint8_t CommInit = 0;

// Текущая команда для получения данных.
static uint8_t Command = COMMAND_RDBLI_RLI_ASS;

// Прототипы локальных функций.
static void setup();
static void loop();
static void timers_loop();
static void ecu_communication();
#ifdef AUTO_BRIGHT_PIN
	static uint8_t get_oled_bright();
#endif
static void ignition_monitoring();

#ifdef DEBUG_MODE
	static void debug_mode();
#endif

int main() {
	setup();
	while(1) {
		loop();
	}
	return 0;
}

// Инициализация при включении 
static void setup() {
	wdt_enable(WDTO_500MS);	// Сторожевой собак на 500 мс.

	timers_init();		// Инициализация таймеров.
	counters_init();	// Инициализация счетчиков пробега и расхода топлива.
	uart_init();		// Инициализация UART.

	#ifdef AUTO_BRIGHT_PIN
		adc_init();			// Инициализация АЦП.
	#endif

	#ifdef SPI_CS_PIN
		spi_init();			// Инициализация SPI.
	#else
		i2c_init();			// Инициализация I2C.
	#endif

	SET_PIN_MODE_INPUT(INT_IGN_PIN);	// Проверка ЗЗ.
	SET_PIN_LOW(INT_IGN_PIN);

	#ifdef INT_OIL_PIN					// ДАДМ
		dadm_init();
	#endif

	buttons_init();			// Настройка входов для кнопок.

	#ifdef SPEED_CHIME_PIN
		speed_chime_init();
	#endif

	#ifdef DEBUG_MODE
		SET_PIN_HIGH(INT_IGN_PIN);
		BK.ScreenMode = 0;
		debug_mode();
	#endif
	
	//update_eeprom(0);
	// Считываем данные из EEPROM.
	read_eeprom();

	_delay_ms(200);						// Небольшая пауза перед настройкой дисплея.
	#ifdef AUTO_BRIGHT_PIN
		BK.BrightLCD[0] = get_oled_bright();	// Первоначальная установка яркости.
	#endif
	oled_init(0x3c, BK.BrightLCD[0], 0);	// Настройка OLED, для SPI адрес роли не играет.
}

// Основной цикл
static void loop() {
	timers_loop();
	ecu_communication();

	#ifdef DEBUG_MODE
		debug_mode();
	#endif

	ignition_monitoring();
	if (BK.StartStop == -2) {return;}

	uint16_t UpdatePeriod = LCD_UPDATE_PERIOD;
	if (BK.ScreenMode == 3) {UpdatePeriod = LCD_UPDATE_PERIOD / 2;}
	if (BK.ScreenChange) {UpdatePeriod = CHANGE_ANIMATION_MS;}
	if (BK.StartStop) {UpdatePeriod = START_ANIMATION_MS;}
	
	if (BK.DataStatus == 2 && oled_ready()) {
		if (LCDTimer >= UpdatePeriod) {
			display_draw_data(LCDTimer);
			LCDTimer = 0;
			button_action();
			buttons_clear(); // Сброс необработанных состояний.
		}
	}
	if (LCDTimer >= 2000 && oled_ready()) {
		LCDTimer = 1500;
		display_draw_no_signal();
	}

	#ifdef AUTO_BRIGHT_PIN
		if (AutoBrightTimer >= 50) {
			AutoBrightTimer = 0;
			uint8_t Bright = get_oled_bright();

			if (Bright > BK.BrightLCD[0]) {
				oled_set_bright(++BK.BrightLCD[0]);
			}
			else if (Bright < BK.BrightLCD[0]) {
				oled_set_bright(--BK.BrightLCD[0]);
			}
		}
	#endif

	if (SpeedChimeTimer >= 40) {
		speed_chime_control(SpeedChimeTimer);
		SpeedChimeTimer = 0;
	}
}

static void timers_loop() {
	wdt_reset();		// Сброс сторожевого таймера.

	// Временно выключаем прерывания, чтобы забрать значение счетчика.
	uint8_t TimerAdd = 0;
	cli();
		TimerAdd = MainTimer;
		MainTimer = 0;
	sei();

	#ifdef INT_OIL_PIN
		dadm_test();		// Проверка ДАДМ.
	#endif

	// Счетчики времени.
	if (TimerAdd) {
		LCDTimer += TimerAdd;
		ButtonsTimer += TimerAdd;
		RideTimerMs += TimerAdd;
		AnswerTimer += TimerAdd;
		AutoBrightTimer += TimerAdd;
		SpeedChimeTimer += TimerAdd;
		WaitTimer += 1;

		if (RideTimerMs >= 1000) {
			BK.RideTimer++;
			RideTimerMs -= 1000;
		}

		BK.AlarmBoxTimer += TimerAdd;
		if (BK.AlarmBoxTimer > 800) {BK.AlarmBoxTimer = -800;}

		if (BK.ConfigBoxTimer) {BK.ConfigBoxTimer--;}
	}

	if (ButtonsTimer >= 25) {		// Обработка кнопок.
		ButtonsTimer = 0;
		buttons_update();
	}
}

static void ecu_communication() {
	// Переключение комманд запроса данных.
	uint8_t NewCommand = 0;
	switch (BK.ScreenMode) {
		case 0:	// Получить параметры двигателя.
			NewCommand = COMMAND_RDBLI_RLI_ASS;
			break;
		case 1:	// Получить параметры двигателя.
			NewCommand = COMMAND_RDBLI_RLI_ASS;
			break;
		case 2:	// Получить параметры двигателя.
			NewCommand = COMMAND_RDBLI_RLI_ASS;
			break;
		case 3:	// Получить параметры двигателя.
			NewCommand = COMMAND_RDBLI_RLI_ASS;
			break;			
		case 4:	// Получить параметры двигателя.
			NewCommand = COMMAND_RDBLI_RLI_ASS;
			break;
		case 5:	// Считать ошибки.
			NewCommand = COMMAND_RDTCBS;
			break;			
		case 6:	// Получить значения АЦП.
			NewCommand = COMMAND_RDBLI_RLI_FT;
			break;
	}
	if (Command != NewCommand) {
		Command = NewCommand;
		BK.DataStatus = 0;
	}

	if (AnswerTimer < 0) {return;}
	// Проверка соединения.
	switch (CommInit) {
		case 0:
			if (uart_tx_ready()) {
				uart_send_command(COMMAND_STC);	// Запрос на установку соединения.
				CommInit = 1;
				AnswerTimer = 0;
			}
			break;
		case 1:
			if (uart_get_rx_status() == 2) {					// Пакет принят.
				if (uart_test_rx_packet() == 3) {
					CommInit = 2;	// Соединение установлено.
					AnswerTimer = -100;
				}
			}
			else if (uart_get_rx_status() >= 20) {	// Получен код ошибки.
				CommInit = 0;
				AnswerTimer = -1 * COMM_RESEND_DELAY;
			}
			// Превышено время ожидания ответа.
			if (AnswerTimer > ANSWER_WAIT_TIME && uart_get_rx_status() != 1) {
				CommInit = 0;
				AnswerTimer = -1 * COMM_RESEND_DELAY / 2;
			}
			break;
	}

	if (CommInit != 2) {return;}

	switch (BK.DataStatus) {
		case 0:		// Запрос данных.
			if (uart_tx_ready()) {
				uart_send_command(Command);
				BK.DataStatus = 1;
				AnswerTimer = 0;
			}
			break;
		case 1:		// Ожидание ответа.
			if (uart_get_rx_status() == 2) {		// Пакет принят.
				if (uart_test_rx_packet() == 3) {	// Пакет проверен.
					BK.DataStatus = 2;				// Получен положительный ответ.
				}
			}
			else if (uart_get_rx_status() >= 20) {	// Получен код ошибки.
				BK.DataStatus = 0;
				BK.DataError = uart_get_rx_status();
				if (BK.DataError == 32) {			// Отрицательный ответ ЭБУ.
					CommInit = 0;					// Повторная установка соединения.
					AnswerTimer = -1 * COMM_RESEND_DELAY;
				}
			}
			if (AnswerTimer > ANSWER_WAIT_TIME) {
				BK.DataStatus = 0;
				BK.DataError = 40;
				//CommInit = 0;	// Повторная установка соединения.
				AnswerTimer = -1 * COMM_RESEND_DELAY / 2;
			}
			break;
		case 2:
			// Статус меняется на 3 после обработки данных.
			break;
		case 3:	// Данные обрабработаны.
			BK.DataStatus = 0;
			break;
		case 16:	// Была отправлена внешняя команда.
			BK.DataStatus = 0;
			AnswerTimer = -150;
			break;			
	}
}

#ifdef AUTO_BRIGHT_PIN
	static uint8_t get_oled_bright() {
		int32_t Value = get_adc_value() >> 2;
		Value = (Value - BK.BrightLCD[1]) * (LCD_BRIGHT_MAX - LCD_BRIGHT_MIN) / (BK.BrightLCD[2] - BK.BrightLCD[1]) + LCD_BRIGHT_MIN;
		Value = CONSTRAIN(Value, LCD_BRIGHT_MIN, LCD_BRIGHT_MAX);
		return Value;
	}
#endif

static void ignition_monitoring() {
	if (PIN_READ(INT_IGN_PIN)) {
		if (BK.StartStop == -2) {
			BK.ScreenMode = 0;
			BK.StartStop = 1;
			return;
		}
		else if (BK.ScreenMode == 10 && BK.StartStop == 0) {
			BK.ScreenMode = 0;
			BK.ScreenChange = -1;
			return;
		}
	}
	else {
		if (BK.StartStop == 0) {
			if (BK.ScreenMode == 10) {
				if (WaitTimer > END_SCREEN_TIMER) {BK.StartStop = -1;}
			}
			else {
				update_eeprom(0);
				BK.ScreenMode = 10;
				BK.ScreenChange = 1;
				WaitTimer = 0;
			}
		}
	}
}

#ifdef DEBUG_MODE
static void debug_mode() {
	SET_PIN_HIGH(INT_IGN_PIN);
	BK.DataStatus = 2;

	// set_rx_buffer(7, 0xff);	// Байт флагов текущих неисправностей 1
	// set_rx_buffer(8, 0xff);	// Байт флагов текущих неисправностей 2
	// set_rx_buffer(9, 0xff);	// Байт флагов текущих неисправностей 3
	// set_rx_buffer(10, 0xff);	// Байт флагов текущих неисправностей 4

	set_rx_buffer(11, 126);	// ТОЖ 					N = E - 40 [˚C]
	set_rx_buffer(12, 0);	// АФР 					N = 14.7 * (E + 128) / 256
	set_rx_buffer(13, 0);	// ДПДЗ 				N = E [%]
	set_rx_buffer(14, 9);	// Об/мин 				N = E * 40 [об/мин]
	set_rx_buffer(15, 36);	// Об/мин ХХ 			N = E * 10 [об/мин]
	set_rx_buffer(16, get_adc_value() >> 2);	// (*) Давление			N = E [кПа]
	set_rx_buffer(17, 55);	// РХХ Текущее			N = E [шагов]
	set_rx_buffer(18, 129);	// Коррекция ВП			N = (E + 128) / 256
	set_rx_buffer(19, 48);	// УОЗ					N = E / 2 [гр.КВ] , где E-знаковое
	set_rx_buffer(20, 74);	// Скорость				N = E [км/час]
	set_rx_buffer(21, 141);	// Напряжение пит.		N = 5.2 + E * 0.05 [В]
	set_rx_buffer(22, 19);	// (*) Давление масла	N = ??? [кПа]
	set_rx_buffer(23, 120);	// Напряжение УДК		N = 1.25 * E / 256 [В]
	set_rx_buffer(24, 0);	// Флаги состояния датчика кислорода
	set_rx_buffer(25, 13);	// Длительность импульса впрыска
	set_rx_buffer(26, 85);	// 		N = E / 125 [мсек]
	set_rx_buffer(27, 9);	// Массовый расход воздуха
	set_rx_buffer(28, 51);	// 		N = E / 10 [кг/час]
	set_rx_buffer(29, 4);	// Цикловой расход воздуха
	set_rx_buffer(30, 55);	// 		N = E / 6 [мг/такт]
	set_rx_buffer(31, 1);	// Часовой расход топлива
	set_rx_buffer(32, 255);	// 		N = E / 50 [л/час]
	set_rx_buffer(33, 5);	// Путевой расход топлива
	set_rx_buffer(34, 248);	// 		N = E / 128 [л/100км]
	set_rx_buffer(35, 0);	// Контрольная сумма ПЗУ 
	set_rx_buffer(36, 0);	// Контрольная сумма ПЗУ 
	set_rx_buffer(37, 68);	// Температура воздуха на впуске N = E - 40 [˚C]


	if (BK.ScreenMode == 4) {	// Сохраненные ошибки
		// set_rx_buffer(2, 2);	// Кол-во ошибок

		// set_rx_buffer(3, 0x06);
		// set_rx_buffer(4, 0x03);

		// set_rx_buffer(6, 0x99);
		// set_rx_buffer(7, 0x99);
	}

	if (BK.ScreenMode == 5) {
		// АЦП N = E * 5.0 / 256 [В]
		set_rx_buffer(3, 31);	// АЦП канала детонации
		set_rx_buffer(4, 41);	// АЦП ТОЖ
		set_rx_buffer(5, 51);	// АЦП ДМРВ
		set_rx_buffer(6, 61);	// АЦП напряжения бортсети
		set_rx_buffer(7, 71);	// АЦП УДК
		set_rx_buffer(8, 81);	// АЦП ДПДЗ
		set_rx_buffer(9, 91);	// АЦП ДТВ
	}
}
#endif


// Прерывание при совпадении регистра сравнения OCR0A на таймере 0 каждую 1мс. 
ISR (TIMER0_COMPA_vect) {
	TCNT0 = 0;		// Ручной сброс счетчика.
	MainTimer++;
}

/*
ScreenMode
	Номер активного экрана:
		0 - Основной,
		1 - Вспомогательный,
		2 - Разгон,
		3 - Текущие ошибки,
		4 - Сохраненные ошибки,
		5 - Значения АЦП.

PORTB ^= (1 << 5);
*/