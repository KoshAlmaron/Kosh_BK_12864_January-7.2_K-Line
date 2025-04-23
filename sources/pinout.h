	// Начначение выводов контроллера.

#ifndef _PINOUT_H_
	#define _PINOUT_H_

	// Входы для проверки состояний.
	#define SILENT_MODE_PIN D, 7	// Режим тишины (при подключении диагностического разъема).
	#define INT_OIL_PIN D, 6		// Датчика аварийного давления масла (Опционально).
	#define INT_IGN_PIN D, 3		// Замка зажигания

	// Колокольчик AE86 (Опционально)
	#define SPEED_CHIME_PIN D, 4

	// Кнопки.
	#define BUTTON_UP_PIN C, 3		// Вверх.
	#define BUTTON_DOWN_PIN C, 1	// Вниз.
	#define BUTTON_RIGHT_PIN C, 0	// Вправо.
	#define BUTTON_LEFT_PIN C, 2	// Влево.

	// SPI Дисплей.
	#define SPI_CS_PIN B, 2			// Пин выбора OLED для передачи по SPI.
	#define SPI_DC_PIN B, 1			// Пин выбора команда/данные.
	#define SPI_RESET_PIN D, 5		// Пин RESET.

	// Расскомментировать для работы функции автояркости.
	#define AUTO_BRIGHT_ADC_CHANNEL 7 // Порт АЦП фоторезистора авто яркости.
	
#endif

/*
	0 PD0	(RX)			|	Прием данных от ЭБУ. 							(О)
	1 PD1	(TX)			|	Передача данных к ЭБУ.							(Б/О)
	2 PD2	(INT0)			|	Счетчик импульсов расхода топлива (прерывания).	(Б/З)
   ~3 PD3	(OC2B) (INT1)	|	Вход для проверки состояния замка зажигания.	(С)
	4 PD4					|	Speed Chime (опционально).						(Б/С)
   ~5 PD5	(OC0B)			|	Reset для SPI дисплея на время инициализации.
   ~6 PD6	(OC0A)			|	Вход для проверки состояния ДАДМ.				(З)
	7 PD7					|	Вход проверки режима тишины.					(Б/К)
	8 PB0	(ICP1)			|	Датчик скорости.								(К)
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
	A5 PC5	(ADC5) (SCL)	|
	A6 		(ADC6)			|
	A7		(ADC7)			|	Фоторезистор регулировки яркости.
*/


/*
	SSD1309

1	VSS		GND					Кор
2	VDD		+3.3v				З
3	SCLK	CLK		13	PB5		Б/З
4	SDA		MOSI	11 	PB3		Б/Кор
5	RES		RESET  				Б/З (Полоса)
6	DC		DC		9	PB1		Б/С
7	CS		CS		10	PB2		С
	
*/