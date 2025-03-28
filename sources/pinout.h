	// Начначение выводов контроллера.

#ifndef _PINOUT_H_
	#define _PINOUT_H_

	// Входы для проверки состояний.
	#define INT_OIL_PIN D, 6		// Датчика аварийного давления масла (Опционально).
	#define INT_IGN_PIN D, 7		// Замка зажигания

	// Колокольчик AE86 (Опционально)
	#define SPEED_CHIME_PIN D, 4

	// Кнопки.
	#define BUTTON_UP_PIN C, 3		// Вверх.
	#define BUTTON_DOWN_PIN C, 1	// Вниз.
	#define BUTTON_RIGHT_PIN C, 0	// Вправо.
	#define BUTTON_LEFT_PIN C, 2	// Влево.

	// SPI Дисплей.
	#define SPI_CS_PIN B, 2			// Пин выбора OLED для передачи по SPI.
	#define SPI_DC_PIN B, 1			// Пин выбора комманда/данные.

	// Расскомментировать для работы функции автояркости.
	#define AUTO_BRIGHT_PIN C, 5		// Пин фоторезистора авто яркости (Опционально).
	#define AUTO_BRIGHT_ADC_CHANNEL 5	// Порт АЦП фоторезистора авто яркости.
#endif

/*
	0 PD0	(RX)			|	Прием данных от ЭБУ.
	1 PD1	(TX)			|	Передача данных к ЭБУ.
	2 PD2	(INT0)			|	Счетчик импульсов расхода топлива (прерывания).
   ~3 PD3	(OC2B) (INT1)	|	
	4 PD4					|	Speed Chime (опционально).
   ~5 PD5	(OC0B)			|
   ~6 PD6	(OC0A)			|	Вход для проверки состояния ДАДМ.
	7 PD7					|	Вход для проверки состояния замка зажигания.
	8 PB0	(ICP1)			|	Датчик скорости.
   ~9 PB1	(OC1A)			|	OLED DC.
   ~10 PB2	(OC1B) (SS)		|	SPI OLED CS.
   ~11 PB3	(OC2A) (MOSI)	|	SPI MOSI.
	12 PB4	(MISO)			|	
	13 PB5	(SCL) (LED)		|	SPI SCL.

	A0 PC0	(ADC0)			|	Кнопка вправо.
	A1 PC1	(ADC1)			|	Кнопка вниз.
	A2 PC2	(ADC2)			|	Кнопка влево.
	A3 PC3	(ADC3)			|	Кнопка вверх.
	A4 PC4	(ADC4) (SDA)	|
	A5 PC5	(ADC5) (SCL)	|	Фоторезистор регулировки яркости.
*/


/*
	SSD1309

	1	VSS		GND
	2	VDD		+3.3v
	3	SCLK	CLK		13	PB5
	4	SDA		MOSI	11 	PB3
	5	RES		RESET
	6	DC		DC		9	PB1
	7	CS		CS		10	PB2
	
*/