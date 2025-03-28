//	Хранение всех необходимых параметров.

#ifndef _BKDATA_H_
	#define _BKDATA_H_

	// Структура для хранения переменных.
	typedef struct BK_t {
		uint8_t DataStatus;				// Cтатус получения данных для каждой команды.
		uint8_t DataError;				// Ошибка получения данных (uart_get_rx_status()).
		
		uint32_t DistRide;				// Пройденное расстояние в метрах за время работы.
		uint32_t FuelRide;				// Израсходованно топлива за поездку в мл.
		uint32_t DistDay;				// Пробег суточный.
		uint32_t FuelDay;				// Расход топлива суточный.
		uint32_t DistAll;				// Пробег общий.
		uint32_t FuelAll;				// Расход топлива общий.
		uint16_t RideTimer;				// Время работы за поездку.
		uint8_t FuelConsumptionRatio;	// Коэффициент расхода топлива.
		uint8_t BrightLCD[3];			// Яркость дисплея.
		uint8_t ScreenMode;				// Номер активного экрана.
		int8_t ScreenChange;			// Флаг смены экрана.
		int8_t StartStop;				// Флаг начала/конца работы.

		int16_t AlarmBoxTimer;			// Таймер предупреждающей индикации.
		uint16_t ConfigBoxTimer;		// Таймер блока настройки яркости.
	} BK_t;
	
	extern struct BK_t BK;		// Делаем структуру внешней.

#endif

/*
DataStatus - 
Состояние процесса обмена.
	0 - ожидание,
	1 - отправлена команда,
	2 - получен положительный ответ,
	3 - данные обработаны.

DataError - 
	список тут - uart_get_rx_status(), +
	40 - Превышено время ожидания.

BrightMode
	Режим подсветки, 0 - день, 1 - ночь
BrightLCD
	Яркость подсветки дисплея, 0 - день, 1 - ночь, 2 - буфер для перехода
*/